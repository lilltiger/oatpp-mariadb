#include "EnumTest.hpp"
#include "../utils/EnvLoader.hpp"

#include "oatpp-mariadb/orm.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/Types.hpp"

namespace oatpp { namespace test { namespace mariadb { namespace types {

namespace {

const char* const TAG = "TEST[mariadb::types::EnumTest]";

#include OATPP_CODEGEN_BEGIN(DTO)

class EnumRow : public oatpp::DTO {
  DTO_INIT(EnumRow, DTO)
  DTO_FIELD(String, enum_value);  // ENUM is stored as a string
};

#include OATPP_CODEGEN_END(DTO)

#include OATPP_CODEGEN_BEGIN(DbClient)

class MyClient : public oatpp::orm::DbClient {
public:
  explicit MyClient(const std::shared_ptr<oatpp::orm::Executor>& executor)
    : oatpp::orm::DbClient(executor)
  {}

  QUERY(createTable,
        "CREATE TABLE IF NOT EXISTS `test_enum` ("
        "`enum_value` ENUM('small', 'medium', 'large', 'x-large')"
        ") ENGINE=InnoDB;")

  QUERY(insertValues,
        "INSERT INTO test_enum "
        "(enum_value) "
        "VALUES "
        "(:row.enum_value);",
        PARAM(oatpp::Object<EnumRow>, row))

  QUERY(deleteAll,
        "DELETE FROM test_enum;")

  QUERY(selectAll,
        "SELECT * FROM test_enum;")

  QUERY(insertInvalid,
        "INSERT INTO test_enum (enum_value) VALUES ('invalid');")
};

#include OATPP_CODEGEN_END(DbClient)

}

void EnumTest::onRun() {
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

    // Create the test_enum table
    {
      auto res = client.createTable();
      if (!res->isSuccess()) {
        OATPP_LOGE(TAG, "Failed to create table: %s", res->getErrorMessage()->c_str());
        throw std::runtime_error("Failed to create table");
      }
      OATPP_LOGD(TAG, "Successfully created test_enum table");
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
        auto row = EnumRow::createShared();
        row->enum_value = nullptr;
        auto res = client.insertValues(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted nullptr value");
      }

      // Test 'small' value
      {
        auto row = EnumRow::createShared();
        row->enum_value = "small";
        auto res = client.insertValues(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted 'small' value");
      }

      // Test 'medium' value
      {
        auto row = EnumRow::createShared();
        row->enum_value = "medium";
        auto res = client.insertValues(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted 'medium' value");
      }

      // Test 'large' value
      {
        auto row = EnumRow::createShared();
        row->enum_value = "large";
        auto res = client.insertValues(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted 'large' value");
      }

      // Test 'x-large' value
      {
        auto row = EnumRow::createShared();
        row->enum_value = "x-large";
        auto res = client.insertValues(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted 'x-large' value");
      }

      // Test invalid value (should fail)
      {
        auto res = client.insertInvalid();
        OATPP_ASSERT(!res->isSuccess());
        OATPP_LOGD(TAG, "Verified that invalid enum value is rejected");
      }
    }

    // Verify the results
    {
      auto res = client.selectAll();
      OATPP_ASSERT(res->isSuccess());

      auto dataset = res->fetch<oatpp::Vector<oatpp::Object<EnumRow>>>();
      OATPP_ASSERT(dataset->size() == 5);  // 5 valid values

      // Print results
      oatpp::parser::json::mapping::ObjectMapper om;
      om.getSerializer()->getConfig()->useBeautifier = true;
      auto str = om.writeToString(dataset);
      OATPP_LOGD(TAG, "Query result:\n%s", str->c_str());

      // Verify nullptr value
      {
        auto row = dataset[0];
        OATPP_ASSERT(row->enum_value == nullptr);
      }

      // Verify 'small' value
      {
        auto row = dataset[1];
        OATPP_ASSERT(row->enum_value == "small");
      }

      // Verify 'medium' value
      {
        auto row = dataset[2];
        OATPP_ASSERT(row->enum_value == "medium");
      }

      // Verify 'large' value
      {
        auto row = dataset[3];
        OATPP_ASSERT(row->enum_value == "large");
      }

      // Verify 'x-large' value
      {
        auto row = dataset[4];
        OATPP_ASSERT(row->enum_value == "x-large");
      }

      OATPP_LOGD(TAG, "All assertions passed successfully");
    }

  } catch (const std::exception& e) {
    OATPP_LOGE(TAG, "An error occurred: %s", e.what());
    throw;
  }
}

}}}}
