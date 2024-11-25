#include "EnhancedCrudTest.hpp"
#include "../utils/EnvLoader.hpp"

#include "oatpp-mariadb/orm.hpp"
#include "oatpp-mariadb/TransactionGuard.hpp"
#include "oatpp-mariadb/mapping/JsonHelper.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/Types.hpp"

namespace oatpp { namespace test { namespace mariadb { namespace crud {

namespace {

const char* const TAG = "TEST[mariadb::crud::EnhancedCrudTest]";

#include OATPP_CODEGEN_BEGIN(DTO)

class TestEntity : public oatpp::DTO {
  DTO_INIT(TestEntity, DTO);
  DTO_FIELD(Int32, id);
  DTO_FIELD(String, name, "name");
  DTO_FIELD(String, description, "description");
  DTO_FIELD(Float64, value, "value");
  DTO_FIELD(Int32, quantity, "quantity");
  DTO_FIELD(Boolean, active, "active");
  DTO_FIELD(String, created_at, "created_at");
  DTO_FIELD(String, updated_at, "updated_at");
  DTO_FIELD(String, metadata, "metadata");
};

class CountResult : public oatpp::DTO {
  DTO_INIT(CountResult, DTO);
  DTO_FIELD(Int32, count, "count");
};

class TransactionState : public oatpp::DTO {
  DTO_INIT(TransactionState, DTO);
  DTO_FIELD(String, in_transaction);
};

#include OATPP_CODEGEN_END(DTO)

#include OATPP_CODEGEN_BEGIN(DbClient)

class TestClient : public oatpp::orm::DbClient {
private:
  oatpp::parser::json::mapping::ObjectMapper m_objectMapper;

public:
  TestClient(const std::shared_ptr<oatpp::orm::Executor>& executor)
    : oatpp::orm::DbClient(executor)
    , m_objectMapper()
  {
    setEnabledInterpretations({"DEFAULT", "POSTGRES", "MARIADB"});
    m_objectMapper.getSerializer()->getConfig()->useBeautifier = true;
    oatpp::mariadb::mapping::JsonHelper::setupIntegerSerializers(m_objectMapper);

    // Add Int32 serializer
    m_objectMapper.getSerializer()->setSerializerMethod(data::mapping::type::__class::Int32::CLASS_ID, 
      [](oatpp::parser::json::mapping::Serializer* serializer,
         oatpp::data::stream::ConsistentOutputStream* stream,
         const oatpp::Void& polymorph) {
        if (!polymorph) {
          stream->writeSimple("null", 4);
          return;
        }
        auto value = polymorph.cast<oatpp::Int32>();
        stream->writeAsString((v_int32)value);
      });
  }

  template<typename T>
  oatpp::String serializeToJson(const T& obj) {
    return m_objectMapper.writeToString(obj);
  }

  QUERY(createTable,
        "CREATE TABLE IF NOT EXISTS `test_entities` ("
        "`id` INTEGER PRIMARY KEY AUTO_INCREMENT,"
        "`name` VARCHAR(255) NOT NULL,"
        "`description` TEXT,"
        "`value` DOUBLE NOT NULL DEFAULT 0.0,"
        "`quantity` INTEGER NOT NULL DEFAULT 0,"
        "`active` BOOLEAN DEFAULT TRUE,"
        "`created_at` DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "`updated_at` DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,"
        "`metadata` JSON,"
        "UNIQUE INDEX `idx_name` (`name`)"
        ");")

  QUERY(dropTable,
        "DROP TABLE IF EXISTS `test_entities`;")

  QUERY(createEntity,
        "INSERT INTO `test_entities` "
        "(`name`, `description`, `value`, `quantity`, `active`, `metadata`) "
        "VALUES "
        "(CAST(:entity.name AS CHAR), CAST(:entity.description AS CHAR), "
        "CAST(:entity.value AS DOUBLE), CAST(:entity.quantity AS SIGNED INTEGER), "
        ":entity.active, :entity.metadata) "
        "RETURNING *;",
        PARAM(oatpp::Object<TestEntity>, entity))

  QUERY(updateEntity,
        "UPDATE `test_entities` SET "
        "`name` = CAST(:entity.name AS CHAR), "
        "`description` = CAST(:entity.description AS CHAR), "
        "`value` = CAST(:entity.value AS DOUBLE), "
        "`quantity` = CAST(:entity.quantity AS SIGNED INTEGER), "
        "`active` = :entity.active, "
        "`metadata` = :entity.metadata "
        "WHERE `id` = :id",
        PARAM(oatpp::Int32, id),
        PARAM(oatpp::Object<TestEntity>, entity))

  QUERY(getUpdatedEntity,
        "SELECT * FROM `test_entities` WHERE `id` = :id;",
        PARAM(oatpp::Int32, id))

  QUERY(getEntityById,
        "SELECT * FROM `test_entities` WHERE `id` = :id;",
        PARAM(oatpp::Int32, id))

  QUERY(getEntityByName,
        "SELECT * FROM `test_entities` WHERE `name` = :name;",
        PARAM(oatpp::String, name))

  QUERY(getAllEntities,
        "SELECT * FROM `test_entities` ORDER BY `id`;")

  QUERY(getActiveEntities,
        "SELECT * FROM `test_entities` WHERE `active` = TRUE ORDER BY `id`;")

  QUERY(deleteEntity,
        "DELETE FROM `test_entities` WHERE `id` = :id "
        "RETURNING 1 as count;",
        PARAM(oatpp::Int32, id))

  QUERY(deleteAllEntities,
        "DELETE FROM `test_entities`;")

  QUERY(countEntities,
        "SELECT COUNT(*) as count FROM `test_entities`;")

  QUERY(getCountBeforeDelete,
        "SELECT COUNT(*) as count FROM `test_entities`;")

  QUERY(searchEntities,
        "SELECT * FROM `test_entities` "
        "WHERE `name` LIKE CONCAT('%', :search, '%') "
        "OR `description` LIKE CONCAT('%', :search, '%') "
        "ORDER BY `id`;",
        PARAM(oatpp::String, search))

  QUERY(setIsolationLevel,
        "SET SESSION TRANSACTION ISOLATION LEVEL READ COMMITTED;")

  QUERY(getTransactionState,
        "SELECT IF(@@in_transaction, 'true', 'false') as in_transaction;")
};

#include OATPP_CODEGEN_END(DbClient)

}

void EnhancedCrudTest::onRun() {
  OATPP_LOGD(TAG, "Running Enhanced CRUD Tests...");

  auto env = utils::EnvLoader();
  
  auto options = oatpp::mariadb::ConnectionOptions();
  options.host = env.get("MARIADB_HOST", "127.0.0.1");
  options.port = env.getInt("MARIADB_PORT", 3306);
  options.username = env.get("MARIADB_USER", "root");
  options.password = env.get("MARIADB_PASSWORD", "root");
  options.database = env.get("MARIADB_DATABASE", "test");

  auto connectionProvider = std::make_shared<oatpp::mariadb::ConnectionProvider>(options);
  auto executor = std::make_shared<oatpp::mariadb::Executor>(connectionProvider);
  auto client = TestClient(executor);

  // Setup: create fresh table
  client.dropTable();
  client.createTable();

  // Test 1: Basic CRUD Operations with Transaction
  {
    OATPP_LOGD(TAG, "Test 1: Basic CRUD Operations with Transaction");
    
    auto conn = executor->getConnection();
    OATPP_ASSERT(conn);
    client.setIsolationLevel(conn);
    
    auto beginResult = executor->begin(conn);
    OATPP_ASSERT(beginResult->isSuccess());

    // Create
    auto entity = TestEntity::createShared();
    entity->name = "Test Entity";
    entity->description = "A test entity with transaction";
    entity->value = 123.45;
    entity->quantity = 10;
    entity->active = true;
    entity->metadata = "{\"key\": \"value\"}";

    auto createResult = client.createEntity(entity, conn);
    OATPP_ASSERT(createResult->isSuccess());
    auto createdEntity = createResult->fetch<oatpp::Vector<oatpp::Object<TestEntity>>>()[0];
    OATPP_ASSERT(createdEntity->id > 0);
    OATPP_ASSERT(createdEntity->name == entity->name);

    // Read
    auto readResult = client.getEntityById(createdEntity->id, conn);
    OATPP_ASSERT(readResult->isSuccess());
    auto readEntity = readResult->fetch<oatpp::Vector<oatpp::Object<TestEntity>>>()[0];
    OATPP_ASSERT(readEntity->id == createdEntity->id);
    OATPP_ASSERT(readEntity->name == entity->name);

    // Update
    entity->name = "Updated Entity";
    entity->value = 999.99;
    auto updateResult = client.updateEntity(createdEntity->id, entity, conn);
    OATPP_ASSERT(updateResult->isSuccess());
    
    auto getUpdatedResult = client.getUpdatedEntity(createdEntity->id, conn);
    OATPP_ASSERT(getUpdatedResult->isSuccess());
    auto updatedEntity = getUpdatedResult->fetch<oatpp::Vector<oatpp::Object<TestEntity>>>()[0];
    OATPP_ASSERT(updatedEntity->name == "Updated Entity");
    OATPP_ASSERT(updatedEntity->value == 999.99);

    // Delete
    auto deleteResult = client.deleteEntity(createdEntity->id, conn);
    OATPP_ASSERT(deleteResult->isSuccess());
    auto deleteCount = deleteResult->fetch<oatpp::Vector<oatpp::Object<CountResult>>>()[0];
    OATPP_ASSERT(deleteCount->count == 1);

    auto commitResult = executor->commit(conn);
    OATPP_ASSERT(commitResult->isSuccess());
    executor->closeConnection(conn);
  }

  // Test 2: Batch Operations with Transaction
  {
    OATPP_LOGD(TAG, "Test 2: Batch Operations with Transaction");
    
    auto conn = executor->getConnection();
    OATPP_ASSERT(conn);
    client.setIsolationLevel(conn);
    
    auto beginResult = executor->begin(conn);
    OATPP_ASSERT(beginResult->isSuccess());

    // Create multiple entities
    for(int i = 1; i <= 5; i++) {
      auto entity = TestEntity::createShared();
      entity->name = "Batch Entity " + std::to_string(i);
      entity->description = "Description " + std::to_string(i);
      entity->value = i * 100.0;
      entity->quantity = i * 10;
      entity->active = (i % 2 == 0);
      entity->metadata = "{\"batch\": " + std::to_string(i) + "}";

      auto result = client.createEntity(entity, conn);
      OATPP_ASSERT(result->isSuccess());
    }

    // Verify count
    auto countResult = client.countEntities(conn);
    OATPP_ASSERT(countResult->isSuccess());
    auto count = countResult->fetch<oatpp::Vector<oatpp::Object<CountResult>>>()[0];
    OATPP_ASSERT(count->count == 5);

    // Test search
    auto searchResult = client.searchEntities("Batch", conn);
    OATPP_ASSERT(searchResult->isSuccess());
    auto searchEntities = searchResult->fetch<oatpp::Vector<oatpp::Object<TestEntity>>>();
    OATPP_ASSERT(searchEntities->size() == 5);

    // Test active filter
    auto activeResult = client.getActiveEntities(conn);
    OATPP_ASSERT(activeResult->isSuccess());
    auto activeEntities = activeResult->fetch<oatpp::Vector<oatpp::Object<TestEntity>>>();
    OATPP_ASSERT(activeEntities->size() == 2);

    // Print results
    auto str = client.serializeToJson(searchEntities);

    // Clean up
    auto countBeforeDelete = client.getCountBeforeDelete(conn);
    OATPP_ASSERT(countBeforeDelete->isSuccess());
    auto initialCount = countBeforeDelete->fetch<oatpp::Vector<oatpp::Object<CountResult>>>()[0];
    
    auto deleteResult = client.deleteAllEntities(conn);
    OATPP_ASSERT(deleteResult->isSuccess());
    
    auto countAfterDelete = client.countEntities(conn);
    OATPP_ASSERT(countAfterDelete->isSuccess());
    auto finalCount = countAfterDelete->fetch<oatpp::Vector<oatpp::Object<CountResult>>>()[0];
    
    OATPP_ASSERT(finalCount->count == 0);
    OATPP_ASSERT(initialCount->count == 5);

    auto commitResult = executor->commit(conn);
    OATPP_ASSERT(commitResult->isSuccess());
    executor->closeConnection(conn);
  }

  // Test 3: Transaction Rollback
  {
    OATPP_LOGD(TAG, "Test 3: Transaction Rollback");
    
    auto conn = executor->getConnection();
    OATPP_ASSERT(conn);
    client.setIsolationLevel(conn);
    
    auto beginResult = executor->begin(conn);
    OATPP_ASSERT(beginResult->isSuccess());

    // Create an entity
    auto entity = TestEntity::createShared();
    entity->name = "Rollback Test";
    entity->description = "This entity should be rolled back";
    entity->value = 777.77;
    entity->quantity = 7;
    entity->active = true;

    auto createResult = client.createEntity(entity, conn);
    OATPP_ASSERT(createResult->isSuccess());

    // Verify entity exists in transaction
    auto readResult = client.getEntityByName("Rollback Test", conn);
    OATPP_ASSERT(readResult->isSuccess());
    auto entities = readResult->fetch<oatpp::Vector<oatpp::Object<TestEntity>>>();
    OATPP_ASSERT(entities->size() == 1);

    // Rollback transaction
    auto rollbackResult = executor->rollback(conn);
    OATPP_ASSERT(rollbackResult->isSuccess());

    // Verify entity doesn't exist after rollback
    auto conn2 = executor->getConnection();
    OATPP_ASSERT(conn2);
    readResult = client.getEntityByName("Rollback Test", conn2);
    OATPP_ASSERT(readResult->isSuccess());
    entities = readResult->fetch<oatpp::Vector<oatpp::Object<TestEntity>>>();
    OATPP_ASSERT(entities->size() == 0);

    executor->closeConnection(conn);
    executor->closeConnection(conn2);
  }

  OATPP_LOGD(TAG, "Enhanced CRUD Tests completed successfully!");
}

}}}}
