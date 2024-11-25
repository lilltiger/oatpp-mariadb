#include "Int32Test.hpp"
#include "../utils/EnvLoader.hpp"

#include "oatpp-mariadb/orm.hpp"
#include "oatpp-mariadb/mapping/JsonHelper.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/Types.hpp"

namespace oatpp { namespace test { namespace mariadb { namespace types {

namespace {

const char* const TAG = "TEST[mariadb::types::Int32Test]";

#include OATPP_CODEGEN_BEGIN(DTO)

class Int32Row : public oatpp::DTO {
  DTO_INIT(Int32Row, DTO);
  DTO_FIELD(Int32, signed_value);
  DTO_FIELD(UInt32, unsigned_value);
};

#include OATPP_CODEGEN_END(DTO)

#include OATPP_CODEGEN_BEGIN(DbClient)

class MyClient : public oatpp::orm::DbClient {
public:
  MyClient(const std::shared_ptr<oatpp::orm::Executor>& executor)
    : oatpp::orm::DbClient(executor)
  {
    setEnabledInterpretations({"DEFAULT", "POSTGRES"});
  }

  QUERY(createTable,
        "CREATE TABLE IF NOT EXISTS `test_int32` ("
        "`signed_value` INT,"
        "`unsigned_value` INT UNSIGNED"
        ") ENGINE=InnoDB;")

  QUERY(insertValues,
        "INSERT INTO test_int32 "
        "(signed_value, unsigned_value) "
        "VALUES "
        "(:row.signed_value, :row.unsigned_value);",
        PARAM(oatpp::Object<Int32Row>, row))

  QUERY(deleteAll,
        "DELETE FROM test_int32;")

  QUERY(selectAll,
        "SELECT * FROM test_int32;")

};

#include OATPP_CODEGEN_END(DbClient)

}

void Int32Test::onRun() {
  OATPP_LOGD(TAG, "Running Int32 Tests...");

  auto env = oatpp::test::mariadb::utils::EnvLoader();
  
  auto options = oatpp::mariadb::ConnectionOptions();
  options.host = env.get("MARIADB_HOST", "127.0.0.1");
  options.port = env.getInt("MARIADB_PORT", 3306);
  options.username = env.get("MARIADB_USER", "root");
  options.password = env.get("MARIADB_PASSWORD", "root");
  options.database = env.get("MARIADB_DATABASE", "test");

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

    // Create the test_int32 table
    {
      auto res = client.createTable();
      if (!res->isSuccess()) {
        OATPP_LOGE(TAG, "Failed to create table: %s", res->getErrorMessage()->c_str());
        throw std::runtime_error("Failed to create table");
      }
      OATPP_LOGD(TAG, "Successfully created test_int32 table");
    }

    // Clear any existing data
    {
      auto res = client.deleteAll();
      OATPP_ASSERT(res->isSuccess());
      OATPP_LOGD(TAG, "Cleared existing data");
    }

    // Test cases
    {
      // Test nullptr values
      {
        auto row = Int32Row::createShared();
        row->signed_value = nullptr;
        row->unsigned_value = nullptr;
        auto res = client.insertValues(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted nullptr values");
      }

      // Test minimum signed value
      {
        auto row = Int32Row::createShared();
        row->signed_value = std::numeric_limits<int32_t>::min();
        row->unsigned_value = static_cast<uint32_t>(0);
        auto res = client.insertValues(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted minimum signed value");
      }

      // Test maximum signed value
      {
        auto row = Int32Row::createShared();
        row->signed_value = std::numeric_limits<int32_t>::max();
        row->unsigned_value = static_cast<uint32_t>(0);
        auto res = client.insertValues(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted maximum signed value");
      }

      // Test maximum unsigned value
      {
        auto row = Int32Row::createShared();
        row->signed_value = static_cast<int32_t>(0);
        row->unsigned_value = std::numeric_limits<uint32_t>::max();
        auto res = client.insertValues(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted maximum unsigned value");
      }
    }

    // Verify the results
    {
      auto res = client.selectAll();
      OATPP_ASSERT(res->isSuccess());

      auto dataset = res->fetch<oatpp::Vector<oatpp::Object<Int32Row>>>();
      OATPP_ASSERT(dataset->size() == 4);
      OATPP_LOGD(TAG, "Fetched %d rows from database", dataset->size());

      // Print results
      oatpp::parser::json::mapping::ObjectMapper om;
      om.getSerializer()->getConfig()->useBeautifier = true;
      OATPP_LOGD(TAG, "Setting up ObjectMapper with beautifier enabled");
      
      oatpp::mariadb::mapping::JsonHelper::setupIntegerSerializers(om);
      OATPP_LOGD(TAG, "Integer serializers configured");

      // Log each row before serialization
      for(size_t i = 0; i < dataset->size(); i++) {
        auto& row = dataset[i];
        OATPP_LOGD(TAG, "Row[%d] before serialization: signed_value=%s, unsigned_value=%s",
                   i,
                   (row->signed_value ? std::to_string(*(row->signed_value)).c_str() : "null"),
                   (row->unsigned_value ? std::to_string(*(row->unsigned_value)).c_str() : "null"));
      }

      OATPP_LOGD(TAG, "Attempting to serialize dataset");
      auto str = om.writeToString(dataset);
      OATPP_LOGD(TAG, "Serialization successful. Result:\n%s", str->c_str());

      // Verify nullptr values
      {
        auto row = dataset[0];
        OATPP_ASSERT(row->signed_value == nullptr);
        OATPP_ASSERT(row->unsigned_value == nullptr);
      }

      // Verify minimum signed value
      {
        auto row = dataset[1];
        OATPP_ASSERT(row->signed_value == std::numeric_limits<int32_t>::min());
        OATPP_ASSERT(row->unsigned_value == static_cast<uint32_t>(0));
      }

      // Verify maximum signed value
      {
        auto row = dataset[2];
        OATPP_ASSERT(row->signed_value == std::numeric_limits<int32_t>::max());
        OATPP_ASSERT(row->unsigned_value == static_cast<uint32_t>(0));
      }

      // Verify maximum unsigned value
      {
        auto row = dataset[3];
        OATPP_ASSERT(row->signed_value == static_cast<int32_t>(0));
        OATPP_ASSERT(row->unsigned_value == std::numeric_limits<uint32_t>::max());
      }

      OATPP_LOGD(TAG, "All assertions passed successfully");
    }

  } catch (const std::exception& e) {
    OATPP_LOGE(TAG, "An error occurred: %s", e.what());
    throw;
  }
}

}}}}
