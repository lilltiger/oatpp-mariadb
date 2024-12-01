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
    OATPP_ASSERT(connectionProvider);
    
    auto connection = connectionProvider->get();
    OATPP_ASSERT(connection);
    OATPP_LOGD(TAG, "Successfully connected to database");
    
    auto executor = std::make_shared<oatpp::mariadb::Executor>(connectionProvider);
    MyClient client(executor);

    // Create table and verify
    {
      // Drop table if exists first
      auto res = client.dropTable();
      OATPP_ASSERT(res->isSuccess());
      OATPP_LOGD(TAG, "Dropped existing table if any");

      // Create table
      res = client.createTable();
      if (!res->isSuccess()) {
        OATPP_LOGE(TAG, "Failed to create table: %s", res->getErrorMessage()->c_str());
        throw std::runtime_error("Failed to create table");
      }
      OATPP_LOGD(TAG, "Successfully created test table");
    }

    // Delete all records before running tests
    {
      auto res = client.deleteAll();
      OATPP_ASSERT(res->isSuccess());
      OATPP_LOGD(TAG, "Cleared existing data");
    }

    // Test cases
    {
      // Test different types
      {
        // Clear data before test
        auto res = client.deleteAll();
        OATPP_ASSERT(res->isSuccess());

        auto row = CustomTypeRow::createShared();
        row->data = "{\"customer_id\":42,\"name\":\"John Doe\",\"balance\":1234.56,\"is_active\":true,\"age\":30}";
        res = client.insertValue(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted mixed type values");

        // Verify insertion
        res = client.selectAll();
        OATPP_ASSERT(res->isSuccess());
        
        auto dataset = res->fetch<oatpp::Vector<oatpp::Object<CustomTypeRow>>>();
        OATPP_ASSERT(dataset->size() == 1);
        
        auto fetched = dataset[0];
        OATPP_ASSERT(fetched->data == "{\"customer_id\":42,\"name\":\"John Doe\",\"balance\":1234.56,\"is_active\":true,\"age\":30}");
      }

      // Test NULL values
      {
        // Clear data before test
        auto res = client.deleteAll();
        OATPP_ASSERT(res->isSuccess());

        auto row = CustomTypeRow::createShared();
        row->data = "{\"customer_id\":43,\"name\":null,\"balance\":null,\"is_active\":null,\"age\":null}";
        res = client.insertValue(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted NULL values");

        // Verify NULL values
        res = client.selectAll();
        OATPP_ASSERT(res->isSuccess());
        
        auto dataset = res->fetch<oatpp::Vector<oatpp::Object<CustomTypeRow>>>();
        OATPP_ASSERT(dataset->size() == 1);
        
        auto fetched = dataset[0];
        OATPP_ASSERT(fetched->data == "{\"customer_id\":43,\"name\":null,\"balance\":null,\"is_active\":null,\"age\":null}");
      }

      // Test mixed NULL and non-NULL values
      {
        // Clear data before test
        auto res = client.deleteAll();
        OATPP_ASSERT(res->isSuccess());

        auto row = CustomTypeRow::createShared();
        row->data = "{\"customer_id\":45,\"name\":\"Mixed NULL Test\",\"balance\":null,\"is_active\":true,\"age\":null}";
        res = client.insertValue(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted mixed NULL values");

        // Verify mixed values
        res = client.selectAll();
        OATPP_ASSERT(res->isSuccess());
        
        auto dataset = res->fetch<oatpp::Vector<oatpp::Object<CustomTypeRow>>>();
        OATPP_ASSERT(dataset->size() == 1);
        
        auto fetched = dataset[0];
        OATPP_ASSERT(fetched->data == "{\"customer_id\":45,\"name\":\"Mixed NULL Test\",\"balance\":null,\"is_active\":true,\"age\":null}");
      }

      // Test special characters in string
      {
        auto row = CustomTypeRow::createShared();
        row->data = "{\"customer_id\":44,\"name\":\"O'Connor; DROP TABLE students;--\",\"balance\":-0.01,\"is_active\":false,\"age\":0}";
        auto res = client.insertValue(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted special characters");

        // Verify special characters
        res = client.selectAll();
        OATPP_ASSERT(res->isSuccess());
        
        auto dataset = res->fetch<oatpp::Vector<oatpp::Object<CustomTypeRow>>>();
        OATPP_ASSERT(dataset->size() == 1);
        auto fetched = dataset[0];
        OATPP_ASSERT(fetched->data == "{\"customer_id\":44,\"name\":\"O'Connor; DROP TABLE students;--\",\"balance\":-0.01,\"is_active\":false,\"age\":0}");
      }

      // Test update with different types
      {
        auto row = CustomTypeRow::createShared();
        row->data = "{\"customer_id\":42,\"name\":\"Jane Doe\",\"balance\":9999.99,\"is_active\":false,\"age\":31}";
        auto res = client.updateValue(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Updated with different values");

        // Verify update
        res = client.selectAll();
        OATPP_ASSERT(res->isSuccess());
        
        auto dataset = res->fetch<oatpp::Vector<oatpp::Object<CustomTypeRow>>>();
        OATPP_ASSERT(dataset->size() == 1);
        
        auto fetched = dataset[0];
        OATPP_ASSERT(fetched->data == "{\"customer_id\":42,\"name\":\"Jane Doe\",\"balance\":9999.99,\"is_active\":false,\"age\":31}");
      }

      // Print all results
      {
        auto res = client.selectAll();
        OATPP_ASSERT(res->isSuccess());
        
        auto dataset = res->fetch<oatpp::Vector<oatpp::Object<CustomTypeRow>>>();
        OATPP_ASSERT(dataset->size() == 4);  // 4 test cases
        
        // Log each record before serialization
        for(size_t i = 0; i < dataset->size(); i++) {
          auto row = dataset[i];
          OATPP_LOGD(TAG, "Record %d:", i);
          OATPP_LOGD(TAG, "  data: %s", row->data->c_str());
        }

        // Print results
        oatpp::parser::json::mapping::ObjectMapper om;
        om.getSerializer()->getConfig()->useBeautifier = true;
        auto str = om.writeToString(dataset);
        OATPP_LOGD(TAG, "Query result:\n%s", str->c_str());
      }
    }

    // Cleanup
    {
      auto res = client.dropTable();
      OATPP_ASSERT(res->isSuccess());
      OATPP_LOGD(TAG, "Cleaned up test table");
    }

    OATPP_LOGD(TAG, "All tests passed successfully");

  } catch (const std::exception& e) {
    OATPP_LOGE(TAG, "An error occurred: %s", e.what());
    throw;
  }
}

}}}}