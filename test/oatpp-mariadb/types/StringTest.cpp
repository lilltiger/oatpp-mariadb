#include "StringTest.hpp"
#include "../utils/EnvLoader.hpp"

#include "oatpp-mariadb/orm.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/Types.hpp"
#include <sstream>

namespace oatpp { namespace test { namespace mariadb { namespace types {

namespace {

const char* const TAG = "TEST[mariadb::types::StringTest]";

#include OATPP_CODEGEN_BEGIN(DTO)

class StringRow : public oatpp::DTO {
  DTO_INIT(StringRow, DTO)

  // For testing different string types and lengths
  DTO_FIELD(String, char_value);     // CHAR(10)
  DTO_FIELD(String, text_value);     // TEXT
  DTO_FIELD(String, medtext_value);  // MEDIUMTEXT
  DTO_FIELD(String, longtext_value); // LONGTEXT
  DTO_FIELD(String, fixed_value);    // CHAR(50)
};

#include OATPP_CODEGEN_END(DTO)

#include OATPP_CODEGEN_BEGIN(DbClient)

class MyClient : public oatpp::orm::DbClient {
public:
  explicit MyClient(const std::shared_ptr<oatpp::orm::Executor>& executor)
    : oatpp::orm::DbClient(executor)
  {}

  QUERY(createTable,
        "CREATE TABLE IF NOT EXISTS `test_string` ("
        "`char_value` CHAR(10),"
        "`text_value` TEXT,"
        "`medtext_value` MEDIUMTEXT,"
        "`longtext_value` LONGTEXT,"
        "`fixed_value` CHAR(50)"
        ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;")

  QUERY(insertValues,
        "INSERT INTO test_string "
        "(char_value, text_value, medtext_value, longtext_value, fixed_value) "
        "VALUES "
        "(:row.char_value, :row.text_value, :row.medtext_value, :row.longtext_value, :row.fixed_value);",
        PARAM(oatpp::Object<StringRow>, row))

  QUERY(deleteAll,
        "DELETE FROM test_string;")

  QUERY(selectAll,
        "SELECT * FROM test_string;")
};

#include OATPP_CODEGEN_END(DbClient)

}

void StringTest::onRun() {
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

    // Create the test_string table
    {
      auto res = client.createTable();
      if (!res->isSuccess()) {
        OATPP_LOGE(TAG, "Failed to create table: %s", res->getErrorMessage()->c_str());
        throw std::runtime_error("Failed to create table");
      }
      OATPP_LOGD(TAG, "Successfully created test_string table");
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
        auto row = StringRow::createShared();
        row->char_value = nullptr;
        row->text_value = nullptr;
        row->medtext_value = nullptr;
        row->longtext_value = nullptr;
        row->fixed_value = nullptr;
        auto res = client.insertValues(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted nullptr values");
      }

      // Test empty strings
      {
        auto row = StringRow::createShared();
        row->char_value = "";
        row->text_value = "";
        row->medtext_value = "";
        row->longtext_value = "";
        row->fixed_value = "";
        auto res = client.insertValues(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted empty strings");
      }

      // Test fixed-length CHAR with padding
      {
        auto row = StringRow::createShared();
        row->char_value = "test";      // Will be padded to 10 chars
        row->text_value = "Normal text";
        row->medtext_value = "Medium length text";
        row->longtext_value = "Long text";
        row->fixed_value = "Fixed 50"; // Will be padded to 50 chars
        auto res = client.insertValues(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted padded strings");
      }

      // Test maximum length strings for CHAR
      {
        auto row = StringRow::createShared();
        row->char_value = std::string(10, 'X');  // 10 characters
        row->text_value = "Normal text";
        row->medtext_value = "Medium text";
        row->longtext_value = "Long text";
        row->fixed_value = std::string(50, 'Y'); // 50 characters
        auto res = client.insertValues(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted maximum length CHAR strings");
      }

      // Test large text fields
      {
        auto row = StringRow::createShared();
        row->char_value = "CHAR(10)";
        
        // Generate large text content
        std::stringstream text_ss;
        for(int i = 0; i < 100; i++) {
          text_ss << "Line " << i << ": Regular TEXT content.\n";
        }
        row->text_value = text_ss.str();

        std::stringstream medtext_ss;
        for(int i = 0; i < 1000; i++) {
          medtext_ss << "Line " << i << ": MEDIUMTEXT content with some special chars !@#$%^&*()\n";
        }
        row->medtext_value = medtext_ss.str();

        std::stringstream longtext_ss;
        for(int i = 0; i < 10000; i++) {
          longtext_ss << "Line " << i << ": LONGTEXT content with Unicode: 你好世界\n";
        }
        row->longtext_value = longtext_ss.str();
        
        row->fixed_value = "Fixed length test";
        auto res = client.insertValues(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted large text content");
      }

      // Test special characters and Unicode
      {
        auto row = StringRow::createShared();
        row->char_value = "Special!@#";
        row->text_value = u8"Unicode: 你好世界";
        row->medtext_value = "Newlines:\n\rTabs:\t\tSpaces:   End";
        row->longtext_value = "HTML: <div>Test</div>\nJSON: {\"key\": \"value\"}\nSQL: SELECT * FROM table;";
        row->fixed_value = "Mixed: 你好 ABC 123 !@#";
        auto res = client.insertValues(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted special characters and Unicode");
      }
    }

    // Verify the results
    {
      auto res = client.selectAll();
      OATPP_ASSERT(res->isSuccess());

      auto dataset = res->fetch<oatpp::Vector<oatpp::Object<StringRow>>>();
      OATPP_ASSERT(dataset->size() == 6);

      // Print results
      oatpp::parser::json::mapping::ObjectMapper om;
      om.getSerializer()->getConfig()->useBeautifier = true;
      auto str = om.writeToString(dataset);
      OATPP_LOGD(TAG, "Query result:\n%s", str->c_str());

      // Verify nullptr values
      {
        auto row = dataset[0];
        OATPP_ASSERT(row->char_value == nullptr);
        OATPP_ASSERT(row->text_value == nullptr);
        OATPP_ASSERT(row->medtext_value == nullptr);
        OATPP_ASSERT(row->longtext_value == nullptr);
        OATPP_ASSERT(row->fixed_value == nullptr);
      }

      // Verify empty strings
      {
        auto row = dataset[1];
        OATPP_ASSERT(row->char_value == "");
        OATPP_ASSERT(row->text_value == "");
        OATPP_ASSERT(row->medtext_value == "");
        OATPP_ASSERT(row->longtext_value == "");
        OATPP_ASSERT(row->fixed_value == "");
      }

      // Verify padded strings (note: MariaDB trims trailing spaces by default)
      {
        auto row = dataset[2];
        OATPP_ASSERT(row->char_value == "test");
        OATPP_ASSERT(row->text_value == "Normal text");
        OATPP_ASSERT(row->medtext_value == "Medium length text");
        OATPP_ASSERT(row->longtext_value == "Long text");
        OATPP_ASSERT(row->fixed_value == "Fixed 50");
      }

      // Verify maximum length strings
      {
        auto row = dataset[3];
        OATPP_ASSERT(row->char_value == std::string(10, 'X'));
        OATPP_ASSERT(row->text_value == "Normal text");
        OATPP_ASSERT(row->medtext_value == "Medium text");
        OATPP_ASSERT(row->longtext_value == "Long text");
        OATPP_ASSERT(row->fixed_value == std::string(50, 'Y'));
      }

      // Verify large text content (just check if not null and length)
      {
        auto row = dataset[4];
        OATPP_ASSERT(row->char_value == "CHAR(10)");
        OATPP_ASSERT(row->text_value != nullptr && row->text_value->length() > 1000);
        OATPP_ASSERT(row->medtext_value != nullptr && row->medtext_value->length() > 10000);
        OATPP_ASSERT(row->longtext_value != nullptr && row->longtext_value->length() > 100000);
        OATPP_ASSERT(row->fixed_value == "Fixed length test");
      }

      // Verify special characters and Unicode
      {
        auto row = dataset[5];
        OATPP_ASSERT(row->char_value == "Special!@#");
        OATPP_ASSERT(row->text_value == u8"Unicode: 你好世界");
        OATPP_ASSERT(row->medtext_value == "Newlines:\n\rTabs:\t\tSpaces:   End");
        OATPP_ASSERT(row->longtext_value == "HTML: <div>Test</div>\nJSON: {\"key\": \"value\"}\nSQL: SELECT * FROM table;");
        OATPP_ASSERT(row->fixed_value == "Mixed: 你好 ABC 123 !@#");
      }
    }

  } catch (const std::exception& e) {
    OATPP_LOGE(TAG, "An error occurred: %s", e.what());
    throw; // Re-throw the exception to fail the test
  }
}

}}}}
