#include "VarCharTest.hpp"
#include "../utils/EnvLoader.hpp"

#include "oatpp-mariadb/orm.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/Types.hpp"
#include <sstream>

namespace oatpp { namespace test { namespace mariadb { namespace types {

namespace {

const char* const TAG = "TEST[mariadb::types::VarCharTest]";

#include OATPP_CODEGEN_BEGIN(DTO)

class VarCharRow : public oatpp::DTO {
  DTO_INIT(VarCharRow, DTO)
  DTO_FIELD(String, small_varchar);  // VARCHAR(10)
  DTO_FIELD(String, medium_varchar); // VARCHAR(255)
  DTO_FIELD(String, large_varchar);  // TEXT
};

#include OATPP_CODEGEN_END(DTO)

#include OATPP_CODEGEN_BEGIN(DbClient)

class MyClient : public oatpp::orm::DbClient {
public:
  explicit MyClient(const std::shared_ptr<oatpp::orm::Executor>& executor)
    : oatpp::orm::DbClient(executor)
  {}

  QUERY(createTable,
        "CREATE TABLE IF NOT EXISTS `test_varchar` ("
        "`small_varchar` VARCHAR(10),"
        "`medium_varchar` VARCHAR(255),"
        "`large_varchar` TEXT"
        ") ENGINE=InnoDB;")

  QUERY(insertValues,
        "INSERT INTO test_varchar "
        "(small_varchar, medium_varchar, large_varchar) "
        "VALUES "
        "(:row.small_varchar, :row.medium_varchar, :row.large_varchar);",
        PARAM(oatpp::Object<VarCharRow>, row))

  QUERY(deleteAll,
        "DELETE FROM test_varchar;")

  QUERY(selectAll,
        "SELECT * FROM test_varchar;")

};

#include OATPP_CODEGEN_END(DbClient)

}

void VarCharTest::onRun() {
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

    // Create the test_varchar table
    {
      auto res = client.createTable();
      if (!res->isSuccess()) {
        OATPP_LOGE(TAG, "Failed to create table: %s", res->getErrorMessage()->c_str());
        throw std::runtime_error("Failed to create table");
      }
      OATPP_LOGD(TAG, "Successfully created test_varchar table");
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
        auto row = VarCharRow::createShared();
        row->small_varchar = nullptr;
        row->medium_varchar = nullptr;
        row->large_varchar = nullptr;
        auto res = client.insertValues(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted nullptr values");
      }

      // Test empty strings
      {
        auto row = VarCharRow::createShared();
        row->small_varchar = "";
        row->medium_varchar = "";
        row->large_varchar = "";
        auto res = client.insertValues(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted empty strings");
      }

      // Test normal strings
      {
        auto row = VarCharRow::createShared();
        row->small_varchar = "test";
        row->medium_varchar = "This is a medium length string for testing VARCHAR(255)";
        row->large_varchar = "This is a large string";
        auto res = client.insertValues(row);
        OATPP_ASSERT(res->isSuccess());
      }

      // Test maximum length strings
      {
        auto row = VarCharRow::createShared();
        std::string smallStr(10, 'a');  // 10 characters
        std::string mediumStr(255, 'a'); // 255 characters
        std::string largeStr(3000, 'a'); // 3000 characters for TEXT field
        row->small_varchar = smallStr;
        row->medium_varchar = mediumStr;
        row->large_varchar = largeStr;
        auto res = client.insertValues(row);
        OATPP_ASSERT(res->isSuccess());
      }

      // Test special characters and Unicode in TEXT
      {
        auto row = VarCharRow::createShared();
        row->small_varchar = "!@#$%^&*()";
        row->medium_varchar = u8"Unicode: \u4f60\u597d\u4e16\u754c";
        // Create a large TEXT field with mixed content
        std::stringstream ss;
        ss << "Large TEXT with special characters:\n";
        ss << "1. Unicode: " << u8"\u4f60\u597d\u4e16\u754c\n";
        ss << "2. HTML: <div>Test</div>\n";
        ss << "3. JSON: {\"key\": \"value\"}\n";
        ss << "4. SQL: SELECT * FROM table;\n";
        // Add some repeated content to make it larger
        for(int i = 0; i < 1000; i++) {
            ss << "Line " << i << ": Some text with special chars !@#$%^&*()\n";
        }
        row->large_varchar = ss.str();
        auto res = client.insertValues(row);
        OATPP_ASSERT(res->isSuccess());
      }

      // Test special characters
      {
        auto row = VarCharRow::createShared();
        row->small_varchar = "!@#$%^&*()";
        row->medium_varchar = u8"Unicode: \u4f60\u597d\u4e16\u754c";
        row->large_varchar = "Newlines:\n\rTabs:\t\tSpaces:   End";
        auto res = client.insertValues(row);
        OATPP_ASSERT(res->isSuccess());
      }
    }

    // Verify the results
    {
      auto res = client.selectAll();
      OATPP_ASSERT(res->isSuccess());

      auto dataset = res->fetch<oatpp::Vector<oatpp::Object<VarCharRow>>>();
      OATPP_ASSERT(dataset->size() == 6);

      // Print results
      oatpp::parser::json::mapping::ObjectMapper om;
      om.getSerializer()->getConfig()->useBeautifier = true;
      auto str = om.writeToString(dataset);
      OATPP_LOGD(TAG, "Query result:\n%s", str->c_str());

      try {
        // Verify nullptr values
        {
          auto row = dataset[0];
          if (!row) {
            OATPP_LOGE(TAG, "Row is null");
            throw std::runtime_error("Row is null");
          }
          OATPP_ASSERT(row->small_varchar == nullptr);
          OATPP_ASSERT(row->medium_varchar == nullptr);
          OATPP_ASSERT(row->large_varchar == nullptr);
        }

        // Verify empty strings
        {
          auto row = dataset[1];
          if (!row) {
            OATPP_LOGE(TAG, "Row is null");
            throw std::runtime_error("Row is null");
          }
          OATPP_ASSERT(row->small_varchar == "");
          OATPP_ASSERT(row->medium_varchar == "");
          OATPP_ASSERT(row->large_varchar == "");
        }

        // Verify normal strings
        {
          auto row = dataset[2];
          if (!row) {
            OATPP_LOGE(TAG, "Row is null");
            throw std::runtime_error("Row is null");
          }
          OATPP_ASSERT(row->small_varchar == "test");
          OATPP_ASSERT(row->medium_varchar == "This is a medium length string for testing VARCHAR(255)");
          OATPP_ASSERT(row->large_varchar == "This is a large string");
        }

        // Verify maximum length strings
        {
          auto row = dataset[3];
          if (!row) {
            OATPP_LOGE(TAG, "Row is null");
            throw std::runtime_error("Row is null");
          }
          OATPP_ASSERT(std::string(row->small_varchar->c_str()).length() == 10);
          std::string smallStr(row->small_varchar->c_str());
          OATPP_ASSERT(smallStr == std::string(10, 'a'));

          OATPP_ASSERT(std::string(row->medium_varchar->c_str()).length() == 255);
          std::string mediumStr(row->medium_varchar->c_str());
          OATPP_ASSERT(mediumStr == std::string(255, 'a'));

          OATPP_ASSERT(std::string(row->large_varchar->c_str()).length() == 3000);
          std::string largeStr(row->large_varchar->c_str());
          OATPP_ASSERT(largeStr == std::string(3000, 'a'));
        }

        // Verify special characters
        {
          auto row = dataset[4];
          OATPP_ASSERT(row->small_varchar == "!@#$%^&*()");
          OATPP_ASSERT(row->medium_varchar == u8"Unicode: \u4f60\u597d\u4e16\u754c");
          OATPP_ASSERT(std::string(row->large_varchar->c_str()).length() > 1000);
          std::string content(row->large_varchar->c_str());
          OATPP_ASSERT(content.find("Large TEXT with special characters:") != std::string::npos);
          OATPP_ASSERT(content.find("Unicode:") != std::string::npos);
          OATPP_ASSERT(content.find("HTML:") != std::string::npos);
          OATPP_ASSERT(content.find("JSON:") != std::string::npos);
          OATPP_ASSERT(content.find("SQL:") != std::string::npos);
        }

        // Verify special characters and Unicode in TEXT
        {
          auto row = dataset[5];
          OATPP_ASSERT(row->small_varchar == "!@#$%^&*()");
          OATPP_ASSERT(row->medium_varchar == u8"Unicode: \u4f60\u597d\u4e16\u754c");
          OATPP_ASSERT(std::string(row->large_varchar->c_str()).length() > 1000);
          std::string content(row->large_varchar->c_str());
          OATPP_ASSERT(content.find("Newlines:\n\rTabs:\t\tSpaces:   End") != std::string::npos);
        }

        OATPP_LOGD(TAG, "All assertions passed successfully");
      } catch (const std::exception& e) {
        OATPP_LOGE(TAG, "Assertion failed: %s", e.what());
        throw;
      }
    }

  } catch (const std::exception& e) {
    OATPP_LOGE(TAG, "An error occurred: %s", e.what());
    throw;
  }
}

}}}}
