#include "UInt8Test.hpp"
#include "../utils/EnvLoader.hpp"

#include "oatpp-mariadb/orm.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/Types.hpp"

namespace oatpp { namespace test { namespace mariadb { namespace types {

namespace {

const char* const TAG = "TEST[mariadb::types::UInt8Test]";

#include OATPP_CODEGEN_BEGIN(DTO)

class UInt8Row : public oatpp::DTO {

  DTO_INIT(UInt8Row, DTO);

  DTO_FIELD(UInt8, value);

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
        "CREATE TABLE IF NOT EXISTS `test_uint8` ("
        "`value` TINYINT UNSIGNED"
        ") ENGINE=InnoDB;")

  QUERY(insertValue,
        "INSERT INTO test_uint8 "
        "(value) "
        "VALUES "
        "(:row.value);",
        PARAM(oatpp::Object<UInt8Row>, row))

  QUERY(deleteAll,
        "DELETE FROM test_uint8;")

  QUERY(selectAll,
        "SELECT * FROM test_uint8;")

};

#include OATPP_CODEGEN_END(DbClient)

}

void UInt8Test::onRun() {
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

    // Create the test_uint8 table
    {
      auto res = client.createTable();
      if (!res->isSuccess()) {
        OATPP_LOGE(TAG, "Failed to create table: %s", res->getErrorMessage()->c_str());
        throw std::runtime_error("Failed to create table");
      }
      OATPP_LOGD(TAG, "Successfully created test_uint8 table");
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
        auto row = UInt8Row::createShared();
        row->value = nullptr;
        auto res = client.insertValue(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted nullptr value");
      }

      // Test minimum value (0)
      {
        auto row = UInt8Row::createShared();
        row->value = static_cast<v_uint8>(0);
        auto res = client.insertValue(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted minimum value (0)");
      }

      // Test maximum value (255)
      {
        auto row = UInt8Row::createShared();
        row->value = static_cast<v_uint8>(255);
        auto res = client.insertValue(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted maximum value (255)");
      }

      // Test middle value (128)
      {
        auto row = UInt8Row::createShared();
        row->value = static_cast<v_uint8>(128);
        auto res = client.insertValue(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted middle value (128)");
      }
    }

    // Verify the results
    {
      auto res = client.selectAll();
      OATPP_ASSERT(res->isSuccess());

      auto dataset = res->template fetch<oatpp::Vector<oatpp::Object<UInt8Row>>>();
      OATPP_ASSERT(dataset->size() == 4);

      // Print results
      oatpp::parser::json::mapping::ObjectMapper om;
      om.getSerializer()->getConfig()->useBeautifier = true;
      auto str = om.writeToString(dataset);
      OATPP_LOGD(TAG, "Query result:\n%s", str->c_str());

      // Verify nullptr value
      {
        auto row = dataset[0];
        OATPP_ASSERT(row->value == nullptr);
      }

      // Verify minimum value
      {
        auto row = dataset[1];
        OATPP_ASSERT(row->value == static_cast<v_uint8>(0));
      }

      // Verify maximum value
      {
        auto row = dataset[2];
        OATPP_ASSERT(row->value == static_cast<v_uint8>(255));
      }

      // Verify middle value
      {
        auto row = dataset[3];
        OATPP_ASSERT(row->value == static_cast<v_uint8>(128));
      }

      OATPP_LOGD(TAG, "All assertions passed successfully");
    }

  } catch (const std::exception& e) {
    OATPP_LOGE(TAG, "An error occurred: %s", e.what());
    throw;
  }
}

}}}}
