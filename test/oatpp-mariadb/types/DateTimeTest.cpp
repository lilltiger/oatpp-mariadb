#include "DateTimeTest.hpp"
#include "../utils/EnvLoader.hpp"

#include "oatpp-mariadb/orm.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/Types.hpp"

namespace oatpp { namespace test { namespace mariadb { namespace types {

namespace {

const char* const TAG = "TEST[mariadb::types::DateTimeTest]";

#include OATPP_CODEGEN_BEGIN(DTO)

class DateTimeRow : public oatpp::DTO {
  DTO_INIT(DateTimeRow, DTO)
  DTO_FIELD(String, datetime_value);
};

#include OATPP_CODEGEN_END(DTO)

#include OATPP_CODEGEN_BEGIN(DbClient)

class MyClient : public oatpp::orm::DbClient {
public:
  explicit MyClient(const std::shared_ptr<oatpp::orm::Executor>& executor)
    : oatpp::orm::DbClient(executor)
  {}

  QUERY(createTable,
        "CREATE TABLE IF NOT EXISTS `test_datetime` ("
        "`datetime_value` DATETIME"
        ") ENGINE=InnoDB;")

  QUERY(insertValues,
        "INSERT INTO test_datetime "
        "(datetime_value) "
        "VALUES "
        "(:row.datetime_value);",
        PARAM(oatpp::Object<DateTimeRow>, row))

  QUERY(deleteAll,
        "DELETE FROM test_datetime;")

  QUERY(selectAll,
        "SELECT * FROM test_datetime;")

};

#include OATPP_CODEGEN_END(DbClient)

}

void DateTimeTest::onRun() {
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

    // Create the test_datetime table
    {
      auto res = client.createTable();
      if (!res->isSuccess()) {
        OATPP_LOGE(TAG, "Failed to create table: %s", res->getErrorMessage()->c_str());
        throw std::runtime_error("Failed to create table");
      }
      OATPP_LOGD(TAG, "Successfully created test_datetime table");
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
        auto row = DateTimeRow::createShared();
        row->datetime_value = nullptr;
        auto res = client.insertValues(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted nullptr value");
      }

      // Test minimum datetime value
      {
        auto row = DateTimeRow::createShared();
        row->datetime_value = "1000-01-01 00:00:00";
        auto res = client.insertValues(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted minimum datetime value");
      }

      // Test maximum datetime value
      {
        auto row = DateTimeRow::createShared();
        row->datetime_value = "9999-12-31 23:59:59";
        auto res = client.insertValues(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted maximum datetime value");
      }

      // Test current datetime with microseconds
      {
        auto row = DateTimeRow::createShared();
        row->datetime_value = "2023-12-31 23:59:59.999999";
        auto res = client.insertValues(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted current datetime with microseconds");
      }
    }

    // Verify the results
    {
      auto res = client.selectAll();
      OATPP_ASSERT(res->isSuccess());

      auto dataset = res->fetch<oatpp::Vector<oatpp::Object<DateTimeRow>>>();
      OATPP_ASSERT(dataset->size() == 4);

      // Print results
      oatpp::parser::json::mapping::ObjectMapper om;
      om.getSerializer()->getConfig()->useBeautifier = true;
      auto str = om.writeToString(dataset);
      OATPP_LOGD(TAG, "Query result:\n%s", str->c_str());

      // Verify nullptr value
      {
        auto row = dataset[0];
        OATPP_ASSERT(row->datetime_value == nullptr);
      }

      // Verify minimum datetime value
      {
        auto row = dataset[1];
        OATPP_ASSERT(row->datetime_value == "1000-01-01 00:00:00");
      }

      // Verify maximum datetime value
      {
        auto row = dataset[2];
        OATPP_ASSERT(row->datetime_value == "9999-12-31 23:59:59");
      }

      // Verify current datetime value (note: microseconds might be truncated by MariaDB)
      {
        auto row = dataset[3];
        OATPP_ASSERT(row->datetime_value->substr(0, 19) == "2023-12-31 23:59:59");
      }

      OATPP_LOGD(TAG, "All assertions passed successfully");
    }

  } catch (const std::exception& e) {
    OATPP_LOGE(TAG, "An error occurred: %s", e.what());
    throw;
  }
}

}}}}
