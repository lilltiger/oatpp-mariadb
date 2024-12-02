#include "CustomTypeTest.hpp"
#include "../utils/EnvLoader.hpp"

#include "oatpp-mariadb/orm.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/macro/codegen.hpp"

namespace oatpp { namespace test { namespace mariadb { namespace types {

namespace {
  const char* const TAG = "TEST[mariadb::types::CustomTypeTest]";
}

void CustomTypeTest::onRun() {
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

    // Create table and verify
    {
      auto res = client.dropTable();
      OATPP_ASSERT(res->isSuccess());
      OATPP_LOGD(TAG, "Dropped existing table if any");

      res = client.createTable();
      if (!res->isSuccess()) {
        OATPP_LOGE(TAG, "Failed to create table: %s", res->getErrorMessage()->c_str());
        throw std::runtime_error("Failed to create table");
      }
      OATPP_LOGD(TAG, "Successfully created test table");
    }

    // Clear any existing data
    {
      auto res = client.deleteAll();
      OATPP_ASSERT(res->isSuccess());
      OATPP_LOGD(TAG, "Cleared existing data");
    }

    // Test cases
    {
      // Test case 1: Mixed type values
      {
        auto row = CustomTypeRow::createShared();
        row->data = "{\"customer_id\":42,\"name\":\"John Doe\",\"balance\":1234.56,\"is_active\":true,\"age\":30}";
        auto res = client.insertValue(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted mixed type values");
      }

      // Test case 2: NULL values
      {
        auto row = CustomTypeRow::createShared();
        row->data = "{\"customer_id\":43,\"name\":null,\"balance\":null,\"is_active\":null,\"age\":null}";
        auto res = client.insertValue(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted NULL values");
      }

      // Test case 3: Mixed NULL and non-NULL values
      {
        auto row = CustomTypeRow::createShared();
        row->data = "{\"customer_id\":45,\"name\":\"Mixed NULL Test\",\"balance\":null,\"is_active\":true,\"age\":null}";
        auto res = client.insertValue(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted mixed NULL values");
      }

      // Test case 4: Special characters in string
      {
        auto row = CustomTypeRow::createShared();
        row->data = "{\"customer_id\":44,\"name\":\"O'Connor; DROP TABLE students;--\",\"balance\":-0.01,\"is_active\":false,\"age\":0}";
        auto res = client.insertValue(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted special characters");
      }
    }

    // Verify all test cases
    {
      auto res = client.selectAll();
      OATPP_ASSERT(res->isSuccess());
      
      auto dataset = res->fetch<oatpp::Vector<oatpp::Object<CustomTypeRow>>>();
      OATPP_ASSERT(dataset->size() == 4);
      OATPP_LOGD(TAG, "Fetched %d rows from database", dataset->size());

      // Print results
      oatpp::parser::json::mapping::ObjectMapper om;
      om.getSerializer()->getConfig()->useBeautifier = true;
      OATPP_LOGD(TAG, "Setting up ObjectMapper with beautifier enabled");

      // Log each row before serialization
      for(size_t i = 0; i < dataset->size(); i++) {
        auto& row = dataset[i];
        OATPP_LOGD(TAG, "Row[%d] data: %s", i, row->data->c_str());
      }

      OATPP_LOGD(TAG, "Attempting to serialize dataset");
      auto str = om.writeToString(dataset);
      OATPP_LOGD(TAG, "Serialization successful. Result:\n%s", str->c_str());

      // Verify test case 1: Mixed type values
      {
        auto row = dataset[0];
        OATPP_ASSERT(row->data == "{\"customer_id\":42,\"name\":\"John Doe\",\"balance\":1234.56,\"is_active\":true,\"age\":30}");
      }

      // Verify test case 2: NULL values
      {
        auto row = dataset[1];
        OATPP_ASSERT(row->data == "{\"customer_id\":43,\"name\":null,\"balance\":null,\"is_active\":null,\"age\":null}");
      }

      // Verify test case 3: Mixed NULL values
      {
        auto row = dataset[2];
        OATPP_ASSERT(row->data == "{\"customer_id\":45,\"name\":\"Mixed NULL Test\",\"balance\":null,\"is_active\":true,\"age\":null}");
      }

      // Verify test case 4: Special characters
      {
        auto row = dataset[3];
        OATPP_ASSERT(row->data == "{\"customer_id\":44,\"name\":\"O'Connor; DROP TABLE students;--\",\"balance\":-0.01,\"is_active\":false,\"age\":0}");
      }

      OATPP_LOGD(TAG, "All assertions passed successfully");
    }

    // Cleanup
    {
      auto res = client.dropTable();
      OATPP_ASSERT(res->isSuccess());
      OATPP_LOGD(TAG, "Cleaned up test table");
    }

  } catch (const std::exception& e) {
    OATPP_LOGE(TAG, "An error occurred: %s", e.what());
    throw;
  }
}

}}}}