#include "Float64Test.hpp"
#include "../utils/EnvLoader.hpp"

#include "oatpp-mariadb/orm.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/Types.hpp"

namespace oatpp { namespace test { namespace mariadb { namespace types {

namespace {

const char* const TAG = "TEST[mariadb::types::Float64Test]";

#include OATPP_CODEGEN_BEGIN(DTO)

class Float64Row : public oatpp::DTO {

  DTO_INIT(Float64Row, DTO);

  DTO_FIELD(Float64, value);
  DTO_FIELD(Float64, value_nullable);

};

#include OATPP_CODEGEN_END(DTO)

#include OATPP_CODEGEN_BEGIN(DbClient)

class MyClient : public oatpp::orm::DbClient {
public:

  MyClient(const std::shared_ptr<oatpp::orm::Executor>& executor)
    : oatpp::orm::DbClient(executor)
  {
  }

  QUERY(createTable,
        "CREATE TABLE IF NOT EXISTS `test_float64` ("
        "`value` DOUBLE NOT NULL,"
        "`value_nullable` DOUBLE"
        ") ENGINE=InnoDB;")

  QUERY(insertValues,
        "INSERT INTO test_float64 "
        "(value, value_nullable) "
        "VALUES "
        "(:row.value, :row.value_nullable);",
        PARAM(oatpp::Object<Float64Row>, row))

  QUERY(deleteAll,
        "DELETE FROM test_float64;")

  QUERY(selectAll,
        "SELECT * FROM test_float64;")

};

#include OATPP_CODEGEN_END(DbClient)

}

void Float64Test::onRun() {
  // Load environment variables from .env file
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

    // Create the test_float64 table
    {
      auto res = client.createTable();
      if (!res->isSuccess()) {
        OATPP_LOGE(TAG, "Failed to create table: %s", res->getErrorMessage()->c_str());
        throw std::runtime_error("Failed to create table");
      }
      OATPP_LOGD(TAG, "Successfully created test_float64 table");
    }

    // Clear any existing data
    {
      auto res = client.deleteAll();
      if (!res->isSuccess()) {
        OATPP_LOGE(TAG, "Failed to clear table: %s", res->getErrorMessage()->c_str());
        throw std::runtime_error("Failed to clear table");
      }
      OATPP_LOGD(TAG, "Successfully cleared test_float64 table");
    }

    // Test inserting and retrieving various float64 values
    {
      OATPP_LOGD(TAG, "Testing float64 values...");

      // Create test data
      auto row1 = Float64Row::createShared();
      row1->value = 123.456;
      row1->value_nullable = 789.012;

      auto row2 = Float64Row::createShared();
      row2->value = -987.654;
      row2->value_nullable = nullptr;

      auto row3 = Float64Row::createShared();
      row3->value = 0.0;
      row3->value_nullable = -0.0;

      // Insert test data
      {
        auto res = client.insertValues(row1);
        OATPP_ASSERT(res->isSuccess());

        res = client.insertValues(row2);
        OATPP_ASSERT(res->isSuccess());

        res = client.insertValues(row3);
        OATPP_ASSERT(res->isSuccess());
      }

      // Retrieve and verify data
      {
        auto res = client.selectAll();
        OATPP_ASSERT(res->isSuccess());

        auto dataset = res->fetch<oatpp::Vector<oatpp::Object<Float64Row>>>();
        OATPP_ASSERT(dataset->size() == 3);

        OATPP_ASSERT(dataset[0]->value == 123.456);
        OATPP_ASSERT(dataset[0]->value_nullable == 789.012);

        OATPP_ASSERT(dataset[1]->value == -987.654);
        OATPP_ASSERT(dataset[1]->value_nullable == nullptr);

        OATPP_ASSERT(dataset[2]->value == 0.0);
        OATPP_ASSERT(dataset[2]->value_nullable == -0.0);
      }

      OATPP_LOGD(TAG, "Float64 tests passed successfully");
    }

  } catch (const std::exception& e) {
    OATPP_LOGE(TAG, "Test failed: %s", e.what());
    throw;
  }
}

}}}}
