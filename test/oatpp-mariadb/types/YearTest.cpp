#include "YearTest.hpp"
#include "../utils/EnvLoader.hpp"

#include "oatpp-mariadb/orm.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/Types.hpp"

namespace oatpp { namespace test { namespace mariadb { namespace types {

namespace {

const char* const TAG = "TEST[mariadb::types::YearTest]";

#include OATPP_CODEGEN_BEGIN(DTO)

class YearRow : public oatpp::DTO {
  DTO_INIT(YearRow, DTO)
  DTO_FIELD(Int16, year_value);  // YEAR type is stored as a 2-byte integer
};

#include OATPP_CODEGEN_END(DTO)

#include OATPP_CODEGEN_BEGIN(DbClient)

class MyClient : public oatpp::orm::DbClient {
public:
  explicit MyClient(const std::shared_ptr<oatpp::orm::Executor>& executor)
    : oatpp::orm::DbClient(executor)
  {}

  QUERY(createTable,
        "CREATE TABLE IF NOT EXISTS `test_year` ("
        "`year_value` YEAR"
        ") ENGINE=InnoDB;")

  QUERY(insertValues,
        "INSERT INTO test_year "
        "(year_value) "
        "VALUES "
        "(:row.year_value);",
        PARAM(oatpp::Object<YearRow>, row))

  QUERY(deleteAll,
        "DELETE FROM test_year;")

  QUERY(selectAll,
        "SELECT * FROM test_year;")
};

#include OATPP_CODEGEN_END(DbClient)

}

void YearTest::onRun() {
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

    // Create the test_year table
    {
      auto res = client.createTable();
      if (!res->isSuccess()) {
        OATPP_LOGE(TAG, "Failed to create table: %s", res->getErrorMessage()->c_str());
        throw std::runtime_error("Failed to create table");
      }
      OATPP_LOGD(TAG, "Successfully created test_year table");
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
        auto row = YearRow::createShared();
        row->year_value = nullptr;
        auto res = client.insertValues(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted nullptr value");
      }

      // Test minimum year value (1901)
      {
        auto row = YearRow::createShared();
        row->year_value = static_cast<int16_t>(1901);
        auto res = client.insertValues(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted minimum year value");
      }

      // Test maximum year value (2155)
      {
        auto row = YearRow::createShared();
        row->year_value = static_cast<int16_t>(2155);
        auto res = client.insertValues(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted maximum year value");
      }

      // Test current year
      {
        auto row = YearRow::createShared();
        row->year_value = static_cast<int16_t>(2024);
        auto res = client.insertValues(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted current year");
      }

      // Test zero year (stored as 0000)
      {
        auto row = YearRow::createShared();
        row->year_value = static_cast<int16_t>(0);
        auto res = client.insertValues(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted zero year");
      }
    }

    // Verify the results
    {
      auto res = client.selectAll();
      OATPP_ASSERT(res->isSuccess());

      auto dataset = res->fetch<oatpp::Vector<oatpp::Object<YearRow>>>();
      OATPP_ASSERT(dataset->size() == 5);

      // Print results
      oatpp::parser::json::mapping::ObjectMapper om;
      om.getSerializer()->getConfig()->useBeautifier = true;
      auto str = om.writeToString(dataset);
      OATPP_LOGD(TAG, "Query result:\n%s", str->c_str());

      // Verify nullptr value
      {
        auto row = dataset[0];
        OATPP_ASSERT(row->year_value == nullptr);
      }

      // Verify minimum year value
      {
        auto row = dataset[1];
        OATPP_ASSERT(row->year_value == static_cast<int16_t>(1901));
      }

      // Verify maximum year value
      {
        auto row = dataset[2];
        OATPP_ASSERT(row->year_value == static_cast<int16_t>(2155));
      }

      // Verify current year
      {
        auto row = dataset[3];
        OATPP_ASSERT(row->year_value == static_cast<int16_t>(2024));
      }

      // Verify zero year
      {
        auto row = dataset[4];
        OATPP_ASSERT(row->year_value == static_cast<int16_t>(0));
      }

      OATPP_LOGD(TAG, "All assertions passed successfully");
    }

  } catch (const std::exception& e) {
    OATPP_LOGE(TAG, "An error occurred: %s", e.what());
    throw;
  }
}

}}}}
