#include "JsonTest.hpp"
#include "../utils/EnvLoader.hpp"

#include "oatpp-mariadb/orm.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/Types.hpp"

namespace oatpp { namespace test { namespace mariadb { namespace types {

namespace {

const char* const TAG = "TEST[mariadb::types::JsonTest]";

#include OATPP_CODEGEN_BEGIN(DTO)

class JsonRow : public oatpp::DTO {
  DTO_INIT(JsonRow, DTO)
  DTO_FIELD(String, json_value);  // JSON
};

#include OATPP_CODEGEN_END(DTO)

#include OATPP_CODEGEN_BEGIN(DbClient)

class MyClient : public oatpp::orm::DbClient {
public:
  explicit MyClient(const std::shared_ptr<oatpp::orm::Executor>& executor)
    : oatpp::orm::DbClient(executor)
  {}

  QUERY(createTable,
        "CREATE TABLE IF NOT EXISTS `test_json` ("
        "`json_value` JSON"
        ") ENGINE=InnoDB;")

  QUERY(insertValues,
        "INSERT INTO test_json "
        "(json_value) "
        "VALUES "
        "(:row.json_value);",
        PARAM(oatpp::Object<JsonRow>, row))

  QUERY(deleteAll,
        "DELETE FROM test_json;")

  QUERY(selectAll,
        "SELECT * FROM test_json;")
};

#include OATPP_CODEGEN_END(DbClient)

}

void JsonTest::onRun() {
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
      OATPP_LOGD(TAG, "Successfully created test_json table");
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
        auto row = JsonRow::createShared();
        row->json_value = nullptr;
        auto res = client.insertValues(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted nullptr value");
      }

      // Test empty object
      {
        auto row = JsonRow::createShared();
        row->json_value = "{}";
        auto res = client.insertValues(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted empty object");
      }

      // Test empty array
      {
        auto row = JsonRow::createShared();
        row->json_value = "[]";
        auto res = client.insertValues(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted empty array");
      }

      // Test simple object
      {
        auto row = JsonRow::createShared();
        row->json_value = "{\"name\":\"John\",\"age\":30}";
        auto res = client.insertValues(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted simple object");
      }

      // Test array with objects
      {
        auto row = JsonRow::createShared();
        row->json_value = "[{\"id\":1,\"value\":\"first\"},{\"id\":2,\"value\":\"second\"}]";
        auto res = client.insertValues(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted array with objects");
      }

      // Test nested object
      {
        auto row = JsonRow::createShared();
        row->json_value = "{\"user\":{\"name\":\"John\",\"address\":{\"city\":\"New York\",\"country\":\"USA\"}}}";
        auto res = client.insertValues(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted nested object");
      }
    }

    // Verify the results
    {
      auto res = client.selectAll();
      OATPP_ASSERT(res->isSuccess());

      auto dataset = res->fetch<oatpp::Vector<oatpp::Object<JsonRow>>>();
      OATPP_ASSERT(dataset->size() == 6);  // 6 test cases

      // Print results
      oatpp::parser::json::mapping::ObjectMapper om;
      om.getSerializer()->getConfig()->useBeautifier = true;
      auto str = om.writeToString(dataset);
      OATPP_LOGD(TAG, "Query result:\n%s", str->c_str());

      // Verify nullptr value
      {
        auto row = dataset[0];
        OATPP_ASSERT(row->json_value == nullptr);
      }

      // Verify empty object
      {
        auto row = dataset[1];
        OATPP_ASSERT(row->json_value == "{}");
      }

      // Verify empty array
      {
        auto row = dataset[2];
        OATPP_ASSERT(row->json_value == "[]");
      }

      // Verify simple object
      {
        auto row = dataset[3];
        OATPP_ASSERT(row->json_value == "{\"name\":\"John\",\"age\":30}");
      }

      // Verify array with objects
      {
        auto row = dataset[4];
        OATPP_ASSERT(row->json_value == "[{\"id\":1,\"value\":\"first\"},{\"id\":2,\"value\":\"second\"}]");
      }

      // Verify nested object
      {
        auto row = dataset[5];
        OATPP_ASSERT(row->json_value == "{\"user\":{\"name\":\"John\",\"address\":{\"city\":\"New York\",\"country\":\"USA\"}}}");
      }

      OATPP_LOGD(TAG, "All assertions passed successfully");
    }

  } catch (const std::exception& e) {
    OATPP_LOGE(TAG, "An error occurred: %s", e.what());
    throw;
  }
}

}}}}

void runJsonTest() {
  OATPP_RUN_TEST(oatpp::test::mariadb::types::JsonTest);
}
