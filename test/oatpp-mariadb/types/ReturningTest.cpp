#include "ReturningTest.hpp"
#include "../utils/EnvLoader.hpp"

#include "oatpp-mariadb/orm.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/Types.hpp"

namespace oatpp { namespace test { namespace mariadb { namespace types {

namespace {

const char* const TAG = "TEST[mariadb::types::ReturningTest]";

#include OATPP_CODEGEN_BEGIN(DTO)

class TestRow : public oatpp::DTO {
  DTO_INIT(TestRow, DTO);

  DTO_FIELD(Int64, id);
  DTO_FIELD(String, name);
  DTO_FIELD(Float64, value);
  DTO_FIELD(Boolean, active);
  DTO_FIELD(Int64, created_at);
};

#include OATPP_CODEGEN_END(DTO)

#include OATPP_CODEGEN_BEGIN(DbClient)

class MyClient : public oatpp::orm::DbClient {
public:
  explicit MyClient(const std::shared_ptr<oatpp::orm::Executor>& executor)
    : oatpp::orm::DbClient(executor)
  {}

  QUERY(dropTableIfExists,
        "DROP TABLE IF EXISTS `test_returning`;")

  QUERY(createTable,
        "CREATE TABLE IF NOT EXISTS `test_returning` ("
        "`id` BIGINT AUTO_INCREMENT PRIMARY KEY,"
        "`name` VARCHAR(255),"
        "`value` DOUBLE,"
        "`active` BOOLEAN,"
        "`created_at` BIGINT"
        ") ENGINE=InnoDB;")

  QUERY(insertReturningAll,
        "INSERT INTO test_returning "
        "(name, value, active, created_at) "
        "VALUES "
        "(:row.name, :row.value, :row.active, UNIX_TIMESTAMP()) "
        "RETURNING id, name, value, active, created_at",
        PARAM(oatpp::Object<TestRow>, row))

  QUERY(insertReturningSpecific,
        "INSERT INTO test_returning "
        "(name, value, active, created_at) "
        "VALUES "
        "(:row.name, :row.value, :row.active, UNIX_TIMESTAMP()) "
        "RETURNING id, created_at",
        PARAM(oatpp::Object<TestRow>, row))

  QUERY(updateRow,
        "UPDATE test_returning "
        "SET value = :newValue "
        "WHERE id = :id",
        PARAM(Int64, id),
        PARAM(Float64, newValue))

  QUERY(getUpdatedRow,
        "SELECT id, name, value, active "
        "FROM test_returning "
        "WHERE id = :id",
        PARAM(Int64, id))

  QUERY(getRowToDelete,
        "SELECT id, name, value, active, created_at "
        "FROM test_returning "
        "WHERE id = :id",
        PARAM(Int64, id))

  QUERY(deleteRow,
        "DELETE FROM test_returning "
        "WHERE id = :id",
        PARAM(Int64, id))

  QUERY(deleteAll,
        "DELETE FROM test_returning;")
};

#include OATPP_CODEGEN_END(DbClient)

}

void ReturningTest::onRun() {
  auto env = oatpp::test::mariadb::utils::EnvLoader();
  
  auto options = oatpp::mariadb::ConnectionOptions();
  options.host = env.get("MARIADB_HOST", "127.0.0.1");
  options.port = env.getInt("MARIADB_PORT", 3306);
  options.username = env.get("MARIADB_USER", "root");
  options.password = env.get("MARIADB_PASSWORD", "root");
  options.database = env.get("MARIADB_DATABASE", "test");

  OATPP_LOGD(TAG, "Connecting to database...");
  auto connectionProvider = std::make_shared<oatpp::mariadb::ConnectionProvider>(options);
  auto executor = std::make_shared<oatpp::mariadb::Executor>(connectionProvider);
  auto client = MyClient(executor);

  // Create JSON object mapper for logging
  oatpp::parser::json::mapping::ObjectMapper om;
  om.getSerializer()->getConfig()->useBeautifier = true;

  // Drop table if exists
  {
    auto res = client.dropTableIfExists();
    if (res->getErrorMessage()) {
      OATPP_LOGE(TAG, "Failed to drop table: %s", res->getErrorMessage()->c_str());
      throw std::runtime_error("Failed to drop table");
    }
    OATPP_LOGD(TAG, "Dropped existing table if it existed");
  }

  // Create table
  {
    auto res = client.createTable();
    if (res->getErrorMessage()) {
      OATPP_LOGE(TAG, "Failed to create table: %s", res->getErrorMessage()->c_str());
      throw std::runtime_error("Failed to create table");
    }
    OATPP_LOGD(TAG, "Created test_returning table");
  }

  // Clear any existing data
  {
    auto res = client.deleteAll();
    if (res->getErrorMessage()) {
      OATPP_LOGE(TAG, "Failed to clear data: %s", res->getErrorMessage()->c_str());
      throw std::runtime_error("Failed to clear data");
    }
    OATPP_LOGD(TAG, "Cleared existing data");
  }

  // Test INSERT RETURNING *
  {
    auto row = TestRow::createShared();
    row->name = "Test Row 1";
    row->value = 123.45;
    row->active = true;

    auto res = client.insertReturningAll(row);
    OATPP_ASSERT(res != nullptr);
    
    // Check for errors
    if (res->isSuccess() == false) {
      OATPP_LOGE(TAG, "Failed to execute INSERT RETURNING query");
      throw std::runtime_error("Failed to execute INSERT RETURNING query");
    }
    
    if (!res->hasMoreToFetch()) {
      OATPP_LOGE(TAG, "No rows returned from INSERT RETURNING");
      throw std::runtime_error("No rows returned from INSERT RETURNING");
    }

    auto returned = res->fetch<oatpp::Object<TestRow>>();
    if (!returned) {
      OATPP_LOGE(TAG, "Failed to fetch returned row");
      throw std::runtime_error("Failed to fetch returned row");
    }

    OATPP_ASSERT(returned->id > 0);
    OATPP_ASSERT(returned->name == row->name);
    OATPP_ASSERT(returned->value == row->value);
    OATPP_ASSERT(returned->active == row->active);
    OATPP_ASSERT(returned->created_at > 0);

    OATPP_LOGD(TAG, "INSERT RETURNING * result:\n%s", om.writeToString(returned)->c_str());
  }

  // Test INSERT RETURNING specific columns
  {
    auto row = TestRow::createShared();
    row->name = "Test Row 2";
    row->value = 678.90;
    row->active = false;

    auto res = client.insertReturningSpecific(row);
    OATPP_ASSERT(res != nullptr);
    
    // Check for errors
    if (res->isSuccess() == false) {
      OATPP_LOGE(TAG, "Failed to execute INSERT RETURNING query");
      throw std::runtime_error("Failed to execute INSERT RETURNING query");
    }
    
    if (!res->hasMoreToFetch()) {
      OATPP_LOGE(TAG, "No rows returned from INSERT RETURNING");
      throw std::runtime_error("No rows returned from INSERT RETURNING");
    }

    auto returned = res->fetch<oatpp::Object<TestRow>>();
    if (!returned) {
      OATPP_LOGE(TAG, "Failed to fetch returned row");
      throw std::runtime_error("Failed to fetch returned row");
    }

    OATPP_ASSERT(returned->id > 0);
    OATPP_ASSERT(returned->created_at > 0);

    OATPP_LOGD(TAG, "INSERT RETURNING specific result:\n%s", om.writeToString(returned)->c_str());
  }

  // Test UPDATE RETURNING
  {
    auto row = TestRow::createShared();
    row->name = "Test Row 3";
    row->value = 100.00;
    row->active = true;

    auto insertRes = client.insertReturningAll(row);
    OATPP_ASSERT(insertRes != nullptr);
    
    // Check for errors
    if (insertRes->isSuccess() == false) {
      OATPP_LOGE(TAG, "Failed to execute INSERT RETURNING query");
      throw std::runtime_error("Failed to execute INSERT RETURNING query");
    }
    
    if (!insertRes->hasMoreToFetch()) {
      OATPP_LOGE(TAG, "No rows returned from INSERT RETURNING");
      throw std::runtime_error("No rows returned from INSERT RETURNING");
    }

    auto inserted = insertRes->fetch<oatpp::Object<TestRow>>();

    auto updateRes = client.updateRow(inserted->id, 200.00);
    OATPP_ASSERT(updateRes != nullptr);
    
    // Check for errors
    if (updateRes->isSuccess() == false) {
      OATPP_LOGE(TAG, "Failed to execute UPDATE query");
      throw std::runtime_error("Failed to execute UPDATE query");
    }

    auto getUpdatedRes = client.getUpdatedRow(inserted->id);
    OATPP_ASSERT(getUpdatedRes != nullptr);
    
    // Check for errors
    if (getUpdatedRes->isSuccess() == false) {
      OATPP_LOGE(TAG, "Failed to execute SELECT query");
      throw std::runtime_error("Failed to execute SELECT query");
    }
    
    if (!getUpdatedRes->hasMoreToFetch()) {
      OATPP_LOGE(TAG, "No rows returned from SELECT");
      throw std::runtime_error("No rows returned from SELECT");
    }

    auto updated = getUpdatedRes->fetch<oatpp::Object<TestRow>>();
    OATPP_ASSERT(updated->id > 0);
    OATPP_ASSERT(updated->name == row->name);
    OATPP_ASSERT(updated->value == 200.00);
    OATPP_ASSERT(updated->active == row->active);

    OATPP_LOGD(TAG, "UPDATE RETURNING result:\n%s", om.writeToString(updated)->c_str());
  }

  // Test DELETE RETURNING
  {
    auto row = TestRow::createShared();
    row->name = "Test Row 4";
    row->value = 300.00;
    row->active = false;

    auto insertRes = client.insertReturningAll(row);
    OATPP_ASSERT(insertRes != nullptr);
    
    // Check for errors
    if (insertRes->isSuccess() == false) {
      OATPP_LOGE(TAG, "Failed to execute INSERT RETURNING query");
      throw std::runtime_error("Failed to execute INSERT RETURNING query");
    }
    
    if (!insertRes->hasMoreToFetch()) {
      OATPP_LOGE(TAG, "No rows returned from INSERT RETURNING");
      throw std::runtime_error("No rows returned from INSERT RETURNING");
    }

    auto inserted = insertRes->fetch<oatpp::Object<TestRow>>();

    auto getRowToDeleteRes = client.getRowToDelete(inserted->id);
    OATPP_ASSERT(getRowToDeleteRes != nullptr);
    
    // Check for errors
    if (getRowToDeleteRes->isSuccess() == false) {
      OATPP_LOGE(TAG, "Failed to execute SELECT query");
      throw std::runtime_error("Failed to execute SELECT query");
    }
    
    if (!getRowToDeleteRes->hasMoreToFetch()) {
      OATPP_LOGE(TAG, "No rows returned from SELECT");
      throw std::runtime_error("No rows returned from SELECT");
    }

    auto deleted = getRowToDeleteRes->fetch<oatpp::Object<TestRow>>();

    auto deleteRes = client.deleteRow(inserted->id);
    OATPP_ASSERT(deleteRes != nullptr);
    
    // Check for errors
    if (deleteRes->isSuccess() == false) {
      OATPP_LOGE(TAG, "Failed to execute DELETE query");
      throw std::runtime_error("Failed to execute DELETE query");
    }

    OATPP_ASSERT(deleted->id == inserted->id);
    OATPP_ASSERT(deleted->name == row->name);
    OATPP_ASSERT(deleted->value == row->value);
    OATPP_ASSERT(deleted->active == row->active);

    OATPP_LOGD(TAG, "DELETE RETURNING result:\n%s", om.writeToString(deleted)->c_str());
  }

  OATPP_LOGD(TAG, "All RETURNING tests completed successfully");
}

}}}}
