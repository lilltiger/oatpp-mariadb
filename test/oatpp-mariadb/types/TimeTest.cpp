#include "TimeTest.hpp"
#include "../utils/EnvLoader.hpp"

#include "oatpp-mariadb/orm.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/Types.hpp"

namespace oatpp { namespace test { namespace mariadb { namespace types {

namespace {

const char* const TAG = "TEST[mariadb::types::TimeTest]";

#include OATPP_CODEGEN_BEGIN(DTO)

class TimeRow : public oatpp::DTO {
  DTO_INIT(TimeRow, DTO)
  DTO_FIELD(String, time_value);  // TIME
};

#include OATPP_CODEGEN_END(DTO)

#include OATPP_CODEGEN_BEGIN(DbClient)

class MyClient : public oatpp::orm::DbClient {
public:
  explicit MyClient(const std::shared_ptr<oatpp::orm::Executor>& executor)
    : oatpp::orm::DbClient(executor)
  {}

  QUERY(createTable,
        "CREATE TABLE IF NOT EXISTS `test_time` ("
        "`time_value` TIME(6)"  // Support microseconds precision
        ") ENGINE=InnoDB;")

  QUERY(insertValues,
        "INSERT INTO test_time "
        "(time_value) "
        "VALUES "
        "(:row.time_value);",
        PARAM(oatpp::Object<TimeRow>, row))

  QUERY(deleteAll,
        "DELETE FROM test_time;")

  QUERY(selectAll,
        "SELECT * FROM test_time;")
};

#include OATPP_CODEGEN_END(DbClient)

}

void TimeTest::onRun() {
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

    // Create the test_time table
    {
      auto res = client.createTable();
      if (!res->isSuccess()) {
        OATPP_LOGE(TAG, "Failed to create table: %s", res->getErrorMessage()->c_str());
        throw std::runtime_error("Failed to create table");
      }
      OATPP_LOGD(TAG, "Successfully created test_time table");
    }

    // Clear any existing data
    {
      auto res = client.deleteAll();
      OATPP_ASSERT(res->isSuccess());
      OATPP_LOGD(TAG, "Cleared existing data");
    }

    // Test cases
    {
      // Test nullptr value
      {
        auto row = TimeRow::createShared();
        row->time_value = nullptr;
        auto res = client.insertValues(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted nullptr value");
      }

      // Test minimum time value
      {
        auto row = TimeRow::createShared();
        row->time_value = "-838:59:59";  // MariaDB minimum TIME value
        auto res = client.insertValues(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted minimum time value");
      }

      // Test maximum time value
      {
        auto row = TimeRow::createShared();
        row->time_value = "838:59:59";  // MariaDB maximum TIME value
        auto res = client.insertValues(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted maximum time value");
      }

      // Test zero time
      {
        auto row = TimeRow::createShared();
        row->time_value = "00:00:00";
        auto res = client.insertValues(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted zero time");
      }

      // Test time with microseconds
      {
        auto row = TimeRow::createShared();
        row->time_value = "12:34:56.789123";
        auto res = client.insertValues(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted time with microseconds");
      }

      // Test negative time
      {
        auto row = TimeRow::createShared();
        row->time_value = "-12:34:56";
        auto res = client.insertValues(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted negative time");
      }
    }

    // Verify the results
    {
      auto res = client.selectAll();
      OATPP_ASSERT(res->isSuccess());

      auto dataset = res->fetch<oatpp::Vector<oatpp::Object<TimeRow>>>();
      OATPP_ASSERT(dataset->size() == 6);

      // Print results
      oatpp::parser::json::mapping::ObjectMapper om;
      om.getSerializer()->getConfig()->useBeautifier = true;
      auto str = om.writeToString(dataset);
      OATPP_LOGD(TAG, "Query result:\n%s", str->c_str());

      // Verify nullptr value
      {
        auto row = dataset[0];
        OATPP_ASSERT(row->time_value == nullptr);
      }

      // Verify minimum time value
      {
        auto row = dataset[1];
        OATPP_ASSERT(row->time_value == "-838:59:59.00000");
      }

      // Verify maximum time value
      {
        auto row = dataset[2];
        OATPP_ASSERT(row->time_value == "838:59:59.000000");
      }

      // Verify zero time
      {
        auto row = dataset[3];
        OATPP_ASSERT(row->time_value == "00:00:00.000000");
      }

      // Verify time with microseconds
      {
        auto row = dataset[4];
        OATPP_ASSERT(row->time_value == "12:34:56.789123");
      }

      // Verify negative time
      {
        auto row = dataset[5];
        OATPP_ASSERT(row->time_value == "-12:34:56.000000");
      }

      OATPP_LOGD(TAG, "All assertions passed successfully");
    }

  } catch (const std::exception& e) {
    OATPP_LOGE(TAG, "An error occurred: %s", e.what());
    throw;
  }
}

}}}}
