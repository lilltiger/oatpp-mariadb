#include "BooleanTest.hpp"
#include "../utils/EnvLoader.hpp"

#include "oatpp-mariadb/orm.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/Types.hpp"

namespace oatpp { namespace test { namespace mariadb { namespace types {

namespace {

const char* const TAG = "TEST[mariadb::types::BooleanTest]";

#include OATPP_CODEGEN_BEGIN(DTO)

class BooleanRow : public oatpp::DTO {

  DTO_INIT(BooleanRow, DTO);

  DTO_FIELD(Boolean, value);

};

#include OATPP_CODEGEN_END(DTO)

#include OATPP_CODEGEN_BEGIN(DbClient)

class MyClient : public oatpp::orm::DbClient {
public:

  explicit MyClient(const std::shared_ptr<oatpp::orm::Executor>& executor)
    : oatpp::orm::DbClient(executor)
  {}

  QUERY(createTable,
        "CREATE TABLE IF NOT EXISTS `test_boolean` ("
        "`value` BOOLEAN"
        ") ENGINE=InnoDB;")

  QUERY(insertValue,
        "INSERT INTO test_boolean "
        "(value) "
        "VALUES "
        "(:row.value);",
        PARAM(oatpp::Object<BooleanRow>, row))

  QUERY(deleteAll,
        "DELETE FROM test_boolean;")

  QUERY(selectAll,
        "SELECT * FROM test_boolean;")

};

#include OATPP_CODEGEN_END(DbClient)

}

void BooleanTest::onRun() {
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

  auto connectionProvider = std::make_shared<oatpp::mariadb::ConnectionProvider>(options);
  auto dbConnection = connectionProvider->get();
  if (!dbConnection) {
    OATPP_LOGE(TAG, "Failed to establish database connection");
    throw std::runtime_error("Database connection failed");
  }
  OATPP_LOGD(TAG, "Successfully connected to database");
  
  auto executor = std::make_shared<oatpp::mariadb::Executor>(connectionProvider);
  auto client = MyClient(executor);

  // Create the test_boolean table
  {
    auto res = client.createTable();
    if (!res->isSuccess()) {
      OATPP_LOGE(TAG, "Failed to create table: %s", res->getErrorMessage()->c_str());
      throw std::runtime_error("Failed to create table");
    }
    OATPP_LOGD(TAG, "Successfully created test_boolean table");
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
      auto row = BooleanRow::createShared();
      row->value = nullptr;
      auto res = client.insertValue(row);
      OATPP_ASSERT(res->isSuccess());
      OATPP_LOGD(TAG, "Inserted nullptr value");
    }

    // Test true value
    {
      auto row = BooleanRow::createShared();
      row->value = true;
      auto res = client.insertValue(row);
      OATPP_ASSERT(res->isSuccess());
      OATPP_LOGD(TAG, "Inserted true value");
    }

    // Test false value
    {
      auto row = BooleanRow::createShared();
      row->value = false;
      auto res = client.insertValue(row);
      OATPP_ASSERT(res->isSuccess());
      OATPP_LOGD(TAG, "Inserted false value");
    }
  }

  // Verify the results
  {
    auto res = client.selectAll();
    OATPP_ASSERT(res->isSuccess());

    auto dataset = res->fetch<oatpp::Vector<oatpp::Object<BooleanRow>>>();
    OATPP_ASSERT(dataset->size() == 3);

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

    // Verify true value
    {
      auto row = dataset[1];
      OATPP_ASSERT(row->value == true);
    }

    // Verify false value
    {
      auto row = dataset[2];
      OATPP_ASSERT(row->value == false);
    }

    OATPP_LOGD(TAG, "All assertions passed successfully");
  }
}

}}}}
