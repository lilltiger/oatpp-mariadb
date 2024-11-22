#include "CrudTest.hpp"
#include "../utils/EnvLoader.hpp"

#include "oatpp-mariadb/orm.hpp"
#include "oatpp-mariadb/TransactionGuard.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/Types.hpp"

namespace oatpp { namespace test { namespace mariadb { namespace crud {

namespace {

const char* const TAG = "TEST[mariadb::crud::CrudTest]";

#include OATPP_CODEGEN_BEGIN(DTO)

class TestUser : public oatpp::DTO {
  DTO_INIT(TestUser, DTO);
  DTO_FIELD(Int32, id);
  DTO_FIELD(String, username, "username");
  DTO_FIELD(String, email, "email");
  DTO_FIELD(String, created_at, "created_at");
  DTO_FIELD(String, updated_at, "updated_at");
  DTO_FIELD(Boolean, active, "active");
};

class CountResult : public oatpp::DTO {
  DTO_INIT(CountResult, DTO);
  DTO_FIELD(Int32, count, "count");
};

class InsertResult : public oatpp::DTO {
  DTO_INIT(InsertResult, DTO);
  DTO_FIELD(Int32, id);
};

#include OATPP_CODEGEN_END(DTO)

#include OATPP_CODEGEN_BEGIN(DbClient)

class TestClient : public oatpp::orm::DbClient {
public:
  TestClient(const std::shared_ptr<oatpp::orm::Executor>& executor)
    : oatpp::orm::DbClient(executor)
  {
    setEnabledInterpretations({"DEFAULT", "POSTGRES"});
  }

  QUERY(createTable,
        "CREATE TABLE IF NOT EXISTS `test_users` ("
        "`id` INTEGER PRIMARY KEY AUTO_INCREMENT,"
        "`username` VARCHAR(255) NOT NULL UNIQUE,"
        "`email` VARCHAR(255) NOT NULL UNIQUE,"
        "`created_at` DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "`updated_at` DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,"
        "`active` BOOLEAN DEFAULT TRUE"
        ");")

  QUERY(dropTable,
        "DROP TABLE IF EXISTS `test_users`;")

  QUERY(createUser,
        "INSERT INTO `test_users` (`username`, `email`, `active`) "
        "VALUES (:username, :email, :active) "
        "RETURNING id;",
        PARAM(oatpp::String, username),
        PARAM(oatpp::String, email),
        PARAM(oatpp::Boolean, active))

  QUERY(updateUser,
        "UPDATE `test_users` SET `username` = :username, `email` = :email, `active` = :active "
        "WHERE `id` = :id "
        "RETURNING ROW_COUNT() as count;",
        PARAM(oatpp::Int32, id),
        PARAM(oatpp::String, username),
        PARAM(oatpp::String, email),
        PARAM(oatpp::Boolean, active))

  QUERY(getUser,
        "SELECT * FROM `test_users` WHERE `id` = :id;",
        PARAM(oatpp::Int32, id))

  QUERY(getUserByEmail,
        "SELECT * FROM `test_users` WHERE `email` = :email;",
        PARAM(oatpp::String, email))

  QUERY(getAllUsers,
        "SELECT * FROM `test_users` ORDER BY `id`;")

  QUERY(deleteUser,
        "DELETE FROM `test_users` WHERE `id` = :id "
        "RETURNING ROW_COUNT() as count;",
        PARAM(oatpp::Int32, id))

  QUERY(deleteAllUsers,
        "DELETE FROM `test_users` "
        "RETURNING ROW_COUNT() as count;")

  QUERY(countUsers,
        "SELECT COUNT(*) as count FROM `test_users`;")
};

#include OATPP_CODEGEN_END(DbClient)

}

void CrudTest::onRun() {
  OATPP_LOGD(TAG, "Running CRUD Tests...");

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

  // Test 1: Create (INSERT)
  {
    OATPP_LOGD(TAG, "Test 1: Create operations");
    
    // Test single insert
    auto createResult = client.createUser("user1", "user1@example.com", true);
    OATPP_ASSERT(createResult->isSuccess());
    auto insertResult = createResult->fetch<oatpp::Vector<oatpp::Object<InsertResult>>>();
    OATPP_ASSERT(insertResult->size() == 1);
    OATPP_ASSERT(insertResult[0]->id > 0);
    
    // Test duplicate handling
    auto duplicateResult = client.createUser("user1", "user1@example.com", true);
    OATPP_ASSERT(!duplicateResult->isSuccess());
    
    // Test multiple inserts
    createResult = client.createUser("user2", "user2@example.com", true);
    OATPP_ASSERT(createResult->isSuccess());
    insertResult = createResult->fetch<oatpp::Vector<oatpp::Object<InsertResult>>>();
    OATPP_ASSERT(insertResult->size() == 1);
    OATPP_ASSERT(insertResult[0]->id > 0);
    
    createResult = client.createUser("user3", "user3@example.com", false);
    OATPP_ASSERT(createResult->isSuccess());
    insertResult = createResult->fetch<oatpp::Vector<oatpp::Object<InsertResult>>>();
    OATPP_ASSERT(insertResult->size() == 1);
    OATPP_ASSERT(insertResult[0]->id > 0);
  }

  // Test 2: Read (SELECT)
  {
    OATPP_LOGD(TAG, "Test 2: Read operations");
    
    // Test get by ID
    auto result = client.getUser(1);
    OATPP_ASSERT(result->isSuccess());
    auto user = result->fetch<oatpp::Vector<oatpp::Object<TestUser>>>();
    OATPP_ASSERT(user->size() == 1);
    OATPP_ASSERT(user[0]->username == "user1");
    
    // Test get by email
    result = client.getUserByEmail("user2@example.com");
    OATPP_ASSERT(result->isSuccess());
    user = result->fetch<oatpp::Vector<oatpp::Object<TestUser>>>();
    OATPP_ASSERT(user->size() == 1);
    OATPP_ASSERT(user[0]->username == "user2");
    
    // Test get all users
    result = client.getAllUsers();
    OATPP_ASSERT(result->isSuccess());
    auto users = result->fetch<oatpp::Vector<oatpp::Object<TestUser>>>();
    OATPP_ASSERT(users->size() == 3);
    
    // Test get non-existent user
    result = client.getUser(999);
    OATPP_ASSERT(result->isSuccess());
    user = result->fetch<oatpp::Vector<oatpp::Object<TestUser>>>();
    OATPP_ASSERT(user->size() == 0);
  }

  // Test 3: Update
  {
    OATPP_LOGD(TAG, "Test 3: Update operations");
    
    // Test successful update
    auto updateResult = client.updateUser(1, "user1_updated", "user1_updated@example.com", true);
    OATPP_ASSERT(updateResult->isSuccess());
    auto affectedRows = updateResult->fetch<oatpp::Vector<oatpp::Object<CountResult>>>();
    OATPP_ASSERT(affectedRows->size() == 1);
    OATPP_ASSERT(affectedRows[0]->count == 1);
    
    // Verify update
    auto result = client.getUser(1);
    OATPP_ASSERT(result->isSuccess());
    auto user = result->fetch<oatpp::Vector<oatpp::Object<TestUser>>>();
    OATPP_ASSERT(user->size() == 1);
    OATPP_ASSERT(user[0]->username == "user1_updated");
    OATPP_ASSERT(user[0]->email == "user1_updated@example.com");
    
    // Test update with duplicate email
    updateResult = client.updateUser(1, "user1_updated", "user2@example.com", true);
    OATPP_ASSERT(!updateResult->isSuccess());
    
    // Test update non-existent user
    updateResult = client.updateUser(999, "nonexistent", "nonexistent@example.com", true);
    OATPP_ASSERT(updateResult->isSuccess());
    affectedRows = updateResult->fetch<oatpp::Vector<oatpp::Object<CountResult>>>();
    OATPP_ASSERT(affectedRows->size() == 1);
    OATPP_ASSERT(affectedRows[0]->count == 0);
  }

  // Test 4: Delete
  {
    OATPP_LOGD(TAG, "Test 4: Delete operations");
    
    // Count initial users
    auto countResult = client.countUsers();
    OATPP_ASSERT(countResult->isSuccess());
    auto initialCount = countResult->fetch<oatpp::Vector<oatpp::Object<CountResult>>>()[0]->count;
    
    // Test single delete
    auto deleteResult = client.deleteUser(1);
    OATPP_ASSERT(deleteResult->isSuccess());
    auto affectedRows = deleteResult->fetch<oatpp::Vector<oatpp::Object<CountResult>>>();
    OATPP_ASSERT(affectedRows->size() == 1);
    OATPP_ASSERT(affectedRows[0]->count == 1);
    
    // Verify delete
    countResult = client.countUsers();
    OATPP_ASSERT(countResult->isSuccess());
    auto newCount = countResult->fetch<oatpp::Vector<oatpp::Object<CountResult>>>()[0]->count;
    OATPP_ASSERT(newCount == initialCount - 1);
    
    // Test delete non-existent user
    deleteResult = client.deleteUser(999);
    OATPP_ASSERT(deleteResult->isSuccess());
    affectedRows = deleteResult->fetch<oatpp::Vector<oatpp::Object<CountResult>>>();
    OATPP_ASSERT(affectedRows->size() == 1);
    OATPP_ASSERT(affectedRows[0]->count == 0);
    
    // Test delete all
    deleteResult = client.deleteAllUsers();
    OATPP_ASSERT(deleteResult->isSuccess());
    
    // Verify all deleted
    countResult = client.countUsers();
    OATPP_ASSERT(countResult->isSuccess());
    newCount = countResult->fetch<oatpp::Vector<oatpp::Object<CountResult>>>()[0]->count;
    OATPP_ASSERT(newCount == 0);
  }

  // Test 5: Transaction CRUD
  {
    OATPP_LOGD(TAG, "Test 5: Transaction CRUD operations");
    
    oatpp::mariadb::TransactionGuard guard(executor, 3);
    auto conn = guard.getConnection();
    
    // Create users in transaction
    auto createResult = client.createUser("tx_user1", "tx_user1@example.com", true, conn);
    OATPP_ASSERT(createResult->isSuccess());
    auto insertResult = createResult->fetch<oatpp::Vector<oatpp::Object<InsertResult>>>();
    OATPP_ASSERT(insertResult->size() == 1);
    OATPP_ASSERT(insertResult[0]->id > 0);
    
    createResult = client.createUser("tx_user2", "tx_user2@example.com", true, conn);
    OATPP_ASSERT(createResult->isSuccess());
    insertResult = createResult->fetch<oatpp::Vector<oatpp::Object<InsertResult>>>();
    OATPP_ASSERT(insertResult->size() == 1);
    OATPP_ASSERT(insertResult[0]->id > 0);
    
    // Update in transaction
    auto updateResult = client.updateUser(1, "tx_user1_updated", "tx_user1_updated@example.com", true, conn);
    OATPP_ASSERT(updateResult->isSuccess());
    
    // Read in transaction
    auto result = client.getAllUsers(conn);
    OATPP_ASSERT(result->isSuccess());
    auto users = result->fetch<oatpp::Vector<oatpp::Object<TestUser>>>();
    OATPP_ASSERT(users->size() == 2);
    
    // Delete in transaction
    auto deleteResult = client.deleteUser(2, conn);
    OATPP_ASSERT(deleteResult->isSuccess());
    
    // Commit transaction
    OATPP_ASSERT(guard.commit());
    
    // Verify final state
    result = client.getAllUsers();
    OATPP_ASSERT(result->isSuccess());
    users = result->fetch<oatpp::Vector<oatpp::Object<TestUser>>>();
    OATPP_ASSERT(users->size() == 1);
    OATPP_ASSERT(users[0]->username == "tx_user1_updated");
  }

  OATPP_LOGD(TAG, "CRUD Tests completed successfully!");
}

}}}}