#include "SetTest.hpp"
#include "../utils/EnvLoader.hpp"

#include "oatpp-mariadb/orm.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/Types.hpp"

namespace oatpp { namespace test { namespace mariadb { namespace types {

namespace {

const char* const TAG = "TEST[mariadb::types::SetTest]";

#include OATPP_CODEGEN_BEGIN(DTO)

class SetRow : public oatpp::DTO {
  DTO_INIT(SetRow, DTO)
  DTO_FIELD(String, set_value);  // SET
};

#include OATPP_CODEGEN_END(DTO)

#include OATPP_CODEGEN_BEGIN(DbClient)

class MyClient : public oatpp::orm::DbClient {
public:
  explicit MyClient(const std::shared_ptr<oatpp::orm::Executor>& executor)
    : oatpp::orm::DbClient(executor)
  {}

  QUERY(createTable,
        "CREATE TABLE IF NOT EXISTS `test_set` ("
        "`set_value` SET('red', 'green', 'blue', 'yellow')"
        ") ENGINE=InnoDB;")

  QUERY(insertValues,
        "INSERT INTO test_set "
        "(set_value) "
        "VALUES "
        "(:row.set_value);",
        PARAM(oatpp::Object<SetRow>, row))

  QUERY(deleteAll,
        "DELETE FROM test_set;")

  QUERY(selectAll,
        "SELECT * FROM test_set;")
};

#include OATPP_CODEGEN_END(DbClient)

}

void SetTest::onRun() {
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
      OATPP_LOGD(TAG, "Successfully created test_set table");
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
        auto row = SetRow::createShared();
        row->set_value = nullptr;
        auto res = client.insertValues(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted nullptr value");
      }

      // Test single value
      {
        auto row = SetRow::createShared();
        row->set_value = "red";
        auto res = client.insertValues(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted single value 'red'");
      }

      // Test multiple values
      {
        auto row = SetRow::createShared();
        row->set_value = "red,blue";
        auto res = client.insertValues(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted multiple values 'red,blue'");
      }

      // Test all values
      {
        auto row = SetRow::createShared();
        row->set_value = "red,green,blue,yellow";
        auto res = client.insertValues(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted all values");
      }
    }

    // Verify the results
    {
      auto res = client.selectAll();
      OATPP_ASSERT(res->isSuccess());

      auto dataset = res->fetch<oatpp::Vector<oatpp::Object<SetRow>>>();
      OATPP_ASSERT(dataset->size() == 4);  // 4 valid values

      // Print results
      oatpp::parser::json::mapping::ObjectMapper om;
      om.getSerializer()->getConfig()->useBeautifier = true;
      om.getSerializer()->getConfig()->includeNullFields = true;

      // Remove custom serializer - using default NULL handling
      auto str = om.writeToString(dataset);
      OATPP_LOGD(TAG, "Query result:\n%s", str->c_str());

      // Verify nullptr value
      {
        auto row = dataset[0];
        OATPP_ASSERT(row->set_value == nullptr);
      }

      // Verify single value
      {
        auto row = dataset[1];
        OATPP_ASSERT(row->set_value == "red");
      }

      // Verify multiple values
      {
        auto row = dataset[2];
        OATPP_ASSERT(row->set_value == "red,blue");
      }

      // Verify all values
      {
        auto row = dataset[3];
        OATPP_ASSERT(row->set_value == "red,green,blue,yellow");
      }

      OATPP_LOGD(TAG, "All assertions passed successfully");
    }

  } catch (const std::exception& e) {
    OATPP_LOGE(TAG, "An error occurred: %s", e.what());
    throw;
  }
}

}}}}

void runSetTest() {
  OATPP_RUN_TEST(oatpp::test::mariadb::types::SetTest);
}
