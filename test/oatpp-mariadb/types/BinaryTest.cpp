#include "BinaryTest.hpp"
#include "../utils/EnvLoader.hpp"

#include "oatpp-mariadb/orm.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/Types.hpp"
#include "oatpp/core/utils/ConversionUtils.hpp"

namespace oatpp { namespace test { namespace mariadb { namespace types {

namespace {

const char* const TAG = "TEST[mariadb::types::BinaryTest]";

#include OATPP_CODEGEN_BEGIN(DTO)

class BinaryRow : public oatpp::DTO {
  DTO_INIT(BinaryRow, DTO)
  DTO_FIELD(String, binary_value);  // BINARY(16)
};

#include OATPP_CODEGEN_END(DTO)

#include OATPP_CODEGEN_BEGIN(DbClient)

class MyClient : public oatpp::orm::DbClient {
public:
  explicit MyClient(const std::shared_ptr<oatpp::orm::Executor>& executor)
    : oatpp::orm::DbClient(executor)
  {}

  QUERY(createTable,
        "CREATE TABLE IF NOT EXISTS `test_binary` ("
        "`binary_value` BINARY(16)"  // Fixed-length binary data
        ") ENGINE=InnoDB;")

  QUERY(insertValues,
        "INSERT INTO test_binary "
        "(binary_value) "
        "VALUES "
        "(:row.binary_value);",
        PARAM(oatpp::Object<BinaryRow>, row))

  QUERY(deleteAll,
        "DELETE FROM test_binary;")

  QUERY(selectAll,
        "SELECT * FROM test_binary;")
};

#include OATPP_CODEGEN_END(DbClient)

}

void BinaryTest::onRun() {
  auto env = oatpp::test::mariadb::utils::EnvLoader();
  
  auto options = oatpp::mariadb::ConnectionOptions();
  options.host = env.get("MARIADB_HOST", "127.0.0.1");
  options.port = env.getInt("MARIADB_PORT", 3306);
  options.username = env.get("MARIADB_USER", "root");
  options.password = env.get("MARIADB_PASSWORD", "root");
  options.database = env.get("MARIADB_DATABASE", "test");

  OATPP_LOGD(TAG, "Attempting to connect to database '%s' on '%s:%d' as user '%s'", 
             options.database.getValue("").c_str(), 
             options.host.getValue("").c_str(), 
             options.port,
             options.username.getValue("").c_str());

  try {
    auto connectionProvider = std::make_shared<oatpp::mariadb::ConnectionProvider>(options);
    auto dbConnection = connectionProvider->get();
    if (!dbConnection) {
      OATPP_LOGE(TAG, "Failed to establish database connection");
      throw std::runtime_error("Database connection failed");
    }
    OATPP_LOGD(TAG, "Successfully connected to database");
    
    auto executor = std::make_shared<oatpp::mariadb::Executor>(connectionProvider);
    auto client = MyClient(executor);

    // Create table
    {
      auto res = client.createTable();
      if (!res->isSuccess()) {
        OATPP_LOGE(TAG, "Failed to create table: %s", res->getErrorMessage()->c_str());
        throw std::runtime_error("Failed to create table");
      }
      OATPP_LOGD(TAG, "Successfully created test_binary table");
    }

    // Delete all records before running tests
    {
      auto res = client.deleteAll();
      OATPP_ASSERT(res->isSuccess());
      OATPP_LOGD(TAG, "Cleared existing data");
    }

    // Test cases
    {
      // Test nullptr value
      {
        auto row = BinaryRow::createShared();
        row->binary_value = nullptr;
        auto res = client.insertValues(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted nullptr value");
      }

      // Test empty binary
      {
        auto row = BinaryRow::createShared();
        row->binary_value = "";
        auto res = client.insertValues(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted empty binary");
      }

      // Test binary with all zeros
      {
        auto row = BinaryRow::createShared();
        std::string zeros(16, '\0');
        row->binary_value = oatpp::String(zeros.c_str(), zeros.length());
        auto res = client.insertValues(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted binary with all zeros");
      }

      // Test binary with sequential bytes
      {
        auto row = BinaryRow::createShared();
        std::string seq;
        for(int i = 0; i < 16; i++) {
          seq += static_cast<char>(i);
        }
        row->binary_value = oatpp::String(seq.c_str(), seq.length());
        auto res = client.insertValues(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted binary with sequential bytes");
      }

      // Test binary with all ones
      {
        auto row = BinaryRow::createShared();
        std::string ones(16, '\xFF');
        row->binary_value = oatpp::String(ones.c_str(), ones.length());
        auto res = client.insertValues(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted binary with all ones");
      }

      // Test binary with mixed values
      {
        auto row = BinaryRow::createShared();
        // Create binary data using vector to avoid null terminator issues
        std::vector<unsigned char> mixed = {
          0x00, 0xFF, 0x0F, 0xF0, 0xAA, 0x55, 0x12, 0x34,
          0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0, 0x11, 0x22
        };
        row->binary_value = oatpp::String(reinterpret_cast<const char*>(mixed.data()), mixed.size());
        auto res = client.insertValues(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted binary with mixed values");
      }
    }

    // Verify the results
    {
      auto res = client.selectAll();
      OATPP_ASSERT(res->isSuccess());

      auto dataset = res->fetch<oatpp::Vector<oatpp::Object<BinaryRow>>>();
      OATPP_ASSERT(dataset->size() == 6);  // 6 test cases

      // Print results (as hex for better visibility)
      oatpp::parser::json::mapping::ObjectMapper om;
      om.getSerializer()->getConfig()->useBeautifier = true;
      
      // Register custom serializer for binary data
      om.getSerializer()->setSerializerMethod(data::mapping::type::__class::String::CLASS_ID,
        [](oatpp::parser::json::mapping::Serializer* serializer,
           oatpp::data::stream::ConsistentOutputStream* stream,
           const oatpp::Void& polymorph) {
          if (!polymorph) {
            stream->writeSimple("null", 4);
            return;
          }
          auto str = polymorph.cast<oatpp::String>();
          if (!str) {
            stream->writeSimple("null", 4);
            return;
          }
          
          // Check if this is binary data by looking for non-printable characters
          bool isBinary = false;
          const char* data = str->c_str();
          v_buff_size size = str->length();
          
          for(v_buff_size i = 0; i < size; i++) {
            unsigned char c = static_cast<unsigned char>(data[i]);
            if(c > 127 || (c < 32 && c != '\t' && c != '\n' && c != '\r')) {
              isBinary = true;
              break;
            }
          }
          
          if (isBinary) {
            // For binary data, output as hex string
            stream->writeSimple("\"", 1);
            for(v_buff_size i = 0; i < size; i++) {
              char buffer[3];
              snprintf(buffer, sizeof(buffer), "%02X", static_cast<unsigned char>(data[i]));
              stream->writeSimple(buffer, 2);
            }
            stream->writeSimple("\"", 1);
          } else {
            // For regular strings, use normal string serialization
            stream->writeSimple("\"", 1);
            for(v_buff_size i = 0; i < size; i++) {
              char c = data[i];
              switch(c) {
                case '"': stream->writeSimple("\\\"", 2); break;
                case '\\': stream->writeSimple("\\\\", 2); break;
                case '\b': stream->writeSimple("\\b", 2); break;
                case '\f': stream->writeSimple("\\f", 2); break;
                case '\n': stream->writeSimple("\\n", 2); break;
                case '\r': stream->writeSimple("\\r", 2); break;
                case '\t': stream->writeSimple("\\t", 2); break;
                default:
                  if (c >= 32) {
                    stream->writeSimple(&c, 1);
                  } else {
                    char buffer[7];
                    snprintf(buffer, sizeof(buffer), "\\u%04X", c);
                    stream->writeSimple(buffer, 6);
                  }
              }
            }
            stream->writeSimple("\"", 1);
          }
        });

      auto str = om.writeToString(dataset);
      OATPP_LOGD(TAG, "Query result:\n%s", str->c_str());

      // Verify nullptr value
      {
        auto row = dataset[0];
        OATPP_ASSERT(row->binary_value == nullptr);
      }

      // Verify empty binary
      {
        auto row = dataset[1];
        OATPP_ASSERT(row->binary_value->length() == 16); // BINARY(16) pads with zeros
        for(v_int32 i = 0; i < 16; i++) {
          OATPP_ASSERT(row->binary_value->c_str()[i] == '\0');
        }
      }

      // Verify binary with all zeros
      {
        auto row = dataset[2];
        OATPP_ASSERT(row->binary_value->length() == 16);
        for(v_int32 i = 0; i < 16; i++) {
          OATPP_ASSERT(row->binary_value->c_str()[i] == '\0');
        }
      }

      // Verify binary with sequential bytes
      {
        auto row = dataset[3];
        OATPP_ASSERT(row->binary_value->length() == 16);
        for(v_int32 i = 0; i < 16; i++) {
          OATPP_ASSERT(static_cast<unsigned char>(row->binary_value->c_str()[i]) == i);
        }
      }

      // Verify binary with all ones
      {
        auto row = dataset[4];
        OATPP_ASSERT(row->binary_value->length() == 16);
        for(v_int32 i = 0; i < 16; i++) {
          OATPP_ASSERT(static_cast<unsigned char>(row->binary_value->c_str()[i]) == 0xFF);
        }
      }

      // Verify binary with mixed values
      {
        auto row = dataset[5];
        OATPP_ASSERT(row->binary_value->length() == 16);
        const unsigned char expected[] = {0x00, 0xFF, 0x0F, 0xF0, 0xAA, 0x55, 0x12, 0x34,
                                       0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0, 0x11, 0x22};
        for(v_int32 i = 0; i < 16; i++) {
          OATPP_ASSERT(static_cast<unsigned char>(row->binary_value->c_str()[i]) == expected[i]);
        }
      }

      OATPP_LOGD(TAG, "All assertions passed successfully");
    }

  } catch (const std::exception& e) {
    OATPP_LOGE(TAG, "An error occurred: %s", e.what());
    throw;
  }
}

}}}}

void runBinaryTest() {
  OATPP_RUN_TEST(oatpp::test::mariadb::types::BinaryTest);
}
