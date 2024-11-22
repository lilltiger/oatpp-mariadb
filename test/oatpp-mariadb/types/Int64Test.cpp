#include "Int64Test.hpp"
#include "../utils/EnvLoader.hpp"

#include "oatpp-mariadb/orm.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/Types.hpp"

namespace oatpp { namespace test { namespace mariadb { namespace types {

namespace {

const char* const TAG = "TEST[mariadb::types::Int64Test]";

#include OATPP_CODEGEN_BEGIN(DTO)

class Int64Row : public oatpp::DTO {

  DTO_INIT(Int64Row, DTO);

  DTO_FIELD(Int64, signed_value);
  DTO_FIELD(UInt64, unsigned_value);

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
        "CREATE TABLE IF NOT EXISTS `test_int64` ("
        "`signed_value` BIGINT,"
        "`unsigned_value` BIGINT UNSIGNED"
        ") ENGINE=InnoDB;")

  QUERY(insertValues,
        "INSERT INTO test_int64 "
        "(signed_value, unsigned_value) "
        "VALUES "
        "(:row.signed_value, :row.unsigned_value);",
        PARAM(oatpp::Object<Int64Row>, row))

  QUERY(deleteAll,
        "DELETE FROM test_int64;")

  QUERY(selectAll,
        "SELECT * FROM test_int64;")

};

#include OATPP_CODEGEN_END(DbClient)

}

void Int64Test::onRun() {
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

    // Create the test_int64 table
    {
      auto res = client.createTable();
      if (!res->isSuccess()) {
        OATPP_LOGE(TAG, "Failed to create table: %s", res->getErrorMessage()->c_str());
        throw std::runtime_error("Failed to create table");
      }
      OATPP_LOGD(TAG, "Successfully created test_int64 table");
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
        auto row = Int64Row::createShared();
        row->signed_value = nullptr;
        row->unsigned_value = nullptr;
        auto res = client.insertValues(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted nullptr values");
      }

      // Test minimum signed value
      {
        auto row = Int64Row::createShared();
        row->signed_value = std::numeric_limits<int64_t>::min();
        row->unsigned_value = static_cast<uint64_t>(0);
        auto res = client.insertValues(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted minimum signed value");
      }

      // Test maximum signed value
      {
        auto row = Int64Row::createShared();
        row->signed_value = std::numeric_limits<int64_t>::max();
        row->unsigned_value = static_cast<uint64_t>(0);
        auto res = client.insertValues(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted maximum signed value");
      }

      // Test maximum unsigned value
      {
        auto row = Int64Row::createShared();
        row->signed_value = static_cast<int64_t>(0);
        row->unsigned_value = std::numeric_limits<uint64_t>::max();
        auto res = client.insertValues(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted maximum unsigned value");
      }
    }

    // Verify the results
    {
      auto res = client.selectAll();
      OATPP_ASSERT(res->isSuccess());

      auto dataset = res->template fetch<oatpp::Vector<oatpp::Object<Int64Row>>>();
      OATPP_ASSERT(dataset->size() == 4);

      // Print results
      oatpp::parser::json::mapping::ObjectMapper om;
      om.getSerializer()->getConfig()->useBeautifier = true;
      auto str = om.writeToString(dataset);
      OATPP_LOGD(TAG, "Query result:\n%s", str->c_str());

      // Verify nullptr values
      {
        auto row = dataset[0];
        OATPP_ASSERT(row->signed_value == nullptr);
        OATPP_ASSERT(row->unsigned_value == nullptr);
      }

      // Verify minimum signed value
      {
        auto row = dataset[1];
        OATPP_ASSERT(row->signed_value == std::numeric_limits<int64_t>::min());
        OATPP_ASSERT(row->unsigned_value == static_cast<uint64_t>(0));
      }

      // Verify maximum signed value
      {
        auto row = dataset[2];
        OATPP_ASSERT(row->signed_value == std::numeric_limits<int64_t>::max());
        OATPP_ASSERT(row->unsigned_value == static_cast<uint64_t>(0));
      }

      // Verify maximum unsigned value
      {
        auto row = dataset[3];
        OATPP_ASSERT(row->signed_value == static_cast<int64_t>(0));
        OATPP_ASSERT(row->unsigned_value == std::numeric_limits<uint64_t>::max());
      }

      OATPP_LOGD(TAG, "All assertions passed successfully");
    }

  } catch (const std::exception& e) {
    OATPP_LOGE(TAG, "An error occurred: %s", e.what());
    throw;
  }
}

}}}}
