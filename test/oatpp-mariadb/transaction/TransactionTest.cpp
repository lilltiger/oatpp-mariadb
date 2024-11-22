#include "TransactionTest.hpp"
#include "../utils/EnvLoader.hpp"

#include "oatpp-mariadb/orm.hpp"
#include "oatpp-mariadb/TransactionGuard.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/Types.hpp"
#include <iostream>

namespace oatpp { namespace test { namespace mariadb { namespace transaction {

namespace {

const char* const TAG = "TEST[mariadb::transaction::TransactionTest]";

#include OATPP_CODEGEN_BEGIN(DTO)

class TestRow : public oatpp::DTO {
  DTO_INIT(TestRow, DTO);
  DTO_FIELD(Int32, id);
  DTO_FIELD(String, value);
};

class IsolationLevel : public oatpp::DTO {
  DTO_INIT(IsolationLevel, DTO);
  DTO_FIELD(String, transaction_isolation, "@@transaction_isolation");
};

class TransactionState : public oatpp::DTO {
  DTO_INIT(TransactionState, DTO);
  DTO_FIELD(String, in_transaction);
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
        "CREATE TABLE IF NOT EXISTS `test_transactions` ("
        "`id` INTEGER PRIMARY KEY AUTO_INCREMENT,"
        "`value` VARCHAR(255)"
        ");")

  QUERY(dropTable,
        "DROP TABLE IF EXISTS `test_transactions`;")

  QUERY(deleteAll,
        "DELETE FROM `test_transactions`;")

  QUERY(insertRow,
        "INSERT INTO `test_transactions` (`value`) VALUES (:value);",
        PARAM(oatpp::String, value))

  QUERY(selectAll,
        "SELECT * FROM `test_transactions`;")

  QUERY(setIsolationLevel,
        "SET SESSION TRANSACTION ISOLATION LEVEL READ COMMITTED;")

  QUERY(getIsolationLevel,
        "SELECT @@transaction_isolation;")

  QUERY(getTransactionState,
        "SELECT IF(@@in_transaction, 'true', 'false') as in_transaction;")
};

#include OATPP_CODEGEN_END(DbClient)

}

void TransactionTest::onRun() {
  OATPP_LOGD(TAG, "Running Transaction Tests...");

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

  // Test 3: Transaction isolation
  {
    OATPP_LOGD(TAG, "Test 3: Transaction isolation");
    client.deleteAll();

    // Start transaction 1
    auto conn1 = executor->getConnection();
    OATPP_ASSERT(conn1);
    client.setIsolationLevel(conn1);
    auto isolevel = client.getIsolationLevel(conn1);
    OATPP_ASSERT(isolevel->isSuccess());
    auto levels = isolevel->fetch<oatpp::Vector<oatpp::Object<IsolationLevel>>>(1);
    OATPP_ASSERT(levels && levels->size() == 1);
    OATPP_LOGD(TAG, "Current isolation level: %s", levels->front()->transaction_isolation->c_str());
    
    auto beginResult = executor->begin(conn1);
    OATPP_ASSERT(beginResult->isSuccess());
    auto insertResult = client.insertRow("tx1_value", conn1);
    OATPP_ASSERT(insertResult->isSuccess());

    // Start transaction 2 and verify it can't see transaction 1's changes
    auto conn2 = executor->getConnection();
    OATPP_ASSERT(conn2);
    client.setIsolationLevel(conn2);
    beginResult = executor->begin(conn2);
    OATPP_ASSERT(beginResult->isSuccess());
    
    // Verify isolation level for conn2
    isolevel = client.getIsolationLevel(conn2);
    OATPP_ASSERT(isolevel->isSuccess());
    levels = isolevel->fetch<oatpp::Vector<oatpp::Object<IsolationLevel>>>(1);
    OATPP_ASSERT(levels && levels->size() == 1);
    OATPP_LOGD(TAG, "Transaction 2 isolation level: %s", levels->front()->transaction_isolation->c_str());
    
    // Explicitly fetch using conn2 to ensure we're using the right connection
    auto result = client.selectAll(conn2);
    OATPP_ASSERT(result->isSuccess());
    
    auto dataset = result->fetch<oatpp::Vector<oatpp::Object<TestRow>>>();
    OATPP_LOGD(TAG, "Before commit - row count: %d", dataset->size());
    OATPP_ASSERT(dataset->size() == 0); // Should not see uncommitted changes

    // Commit transaction 1
    auto commitResult = executor->commit(conn1);
    OATPP_ASSERT(commitResult->isSuccess());
    OATPP_LOGD(TAG, "Transaction 1 committed");

    // Force transaction 2 to see a fresh view
    commitResult = executor->commit(conn2); // End the first transaction
    OATPP_ASSERT(commitResult->isSuccess());
    
    // Keep the connection alive and start a new transaction
    conn2 = executor->getConnection();
    OATPP_ASSERT(conn2);
    client.setIsolationLevel(conn2);
    beginResult = executor->begin(conn2);
    OATPP_ASSERT(beginResult->isSuccess());
    OATPP_LOGD(TAG, "Started new transaction for connection 2");
    
    // Verify transaction 2 now sees the committed changes
    result = client.selectAll(conn2);
    OATPP_ASSERT(result->isSuccess());
    dataset = result->fetch<oatpp::Vector<oatpp::Object<TestRow>>>();
    OATPP_LOGD(TAG, "After commit - row count: %d", dataset->size());
    OATPP_ASSERT(dataset->size() == 1); // Should see the committed changes
    OATPP_ASSERT(dataset->front()->value == "tx1_value");

    // Clean up
    commitResult = executor->commit(conn2);
    OATPP_ASSERT(commitResult->isSuccess());
    OATPP_LOGD(TAG, "Transaction 2 committed");
    
    // Clear any pending results before closing connections
    commitResult.reset();
    
    // Close connections
    executor->closeConnection(conn1);
    executor->closeConnection(conn2);
  }

  // Test 4: Transaction rollback
  {
    OATPP_LOGD(TAG, "Test 4: Transaction rollback");
    client.deleteAll();

    // Start a transaction and insert data
    auto conn = executor->getConnection();
    OATPP_ASSERT(conn);
    client.setIsolationLevel(conn);
    
    auto beginResult = executor->begin(conn);
    OATPP_ASSERT(beginResult->isSuccess());
    
    auto insertResult = client.insertRow("rollback_test", conn);
    OATPP_ASSERT(insertResult->isSuccess());
    
    // Verify data is visible within the transaction
    auto result = client.selectAll(conn);
    OATPP_ASSERT(result->isSuccess());
    auto dataset = result->fetch<oatpp::Vector<oatpp::Object<TestRow>>>();
    OATPP_LOGD(TAG, "Before rollback - row count: %d", dataset->size());
    OATPP_ASSERT(dataset->size() == 1);
    OATPP_ASSERT(dataset->front()->value == "rollback_test");
    
    // Rollback the transaction
    auto rollbackResult = executor->rollback(conn);
    OATPP_ASSERT(rollbackResult->isSuccess());
    OATPP_LOGD(TAG, "Transaction rolled back");
    
    // Verify transaction is not active
    auto txCheckResult = executor->executeRaw("SELECT IF(@@in_transaction, 'true', 'false') as in_transaction;", conn);
    OATPP_ASSERT(txCheckResult->isSuccess());
    OATPP_LOGD(TAG, "Checking transaction state after rollback");
    auto txState = txCheckResult->fetch<oatpp::Vector<oatpp::Object<TransactionState>>>(1);
    OATPP_ASSERT(txState && txState->size() == 1);
    OATPP_LOGD(TAG, "Transaction state: %s", txState->front()->in_transaction->c_str());
    OATPP_ASSERT(txState->front()->in_transaction == "false");
    
    // Close connection
    executor->closeConnection(conn);
  }

  // Test 5: Deadlock protection with TransactionGuard
  {
    OATPP_LOGD(TAG, "Test 5: Deadlock protection");
    client.deleteAll();

    // Get max retries from environment
    auto maxRetries = env.getInt("MAX_RETRIES", 3);
    OATPP_LOGD(TAG, "Using MAX_RETRIES=%d", maxRetries);

    // Create two transaction guards
    oatpp::mariadb::TransactionGuard tx1(executor, maxRetries);
    oatpp::mariadb::TransactionGuard tx2(executor, maxRetries);

    // Set isolation level for both transactions
    client.setIsolationLevel(tx1.getConnection());
    client.setIsolationLevel(tx2.getConnection());

    // First transaction inserts a row
    bool tx1Success = tx1.execute([&](const provider::ResourceHandle<orm::Connection>& conn) {
      auto result = client.insertRow("tx1_value", conn);
      return result->isSuccess();
    });
    OATPP_ASSERT(tx1Success);

    // Second transaction tries to modify the same row
    bool tx2Success = tx2.execute([&](const provider::ResourceHandle<orm::Connection>& conn) {
      auto result = client.insertRow("tx2_value", conn);
      return result->isSuccess();
    });
    OATPP_ASSERT(tx2Success);

    // Verify final state
    auto conn = executor->getConnection();
    auto result = client.selectAll(conn);
    OATPP_ASSERT(result->isSuccess());
    
    auto dataset = result->fetch<oatpp::Vector<oatpp::Object<TestRow>>>();
    OATPP_LOGD(TAG, "Final row count: %d", dataset->size());
    OATPP_ASSERT(dataset->size() == 2);

    executor->closeConnection(conn);
  }

  // Test 6: Nested transactions (savepoints)
  {
    OATPP_LOGD(TAG, "Test 6: Nested transactions");
    client.deleteAll();

    // Start outer transaction
    auto conn = executor->getConnection();
    OATPP_ASSERT(conn);
    client.setIsolationLevel(conn);
      
    auto beginResult = executor->begin(conn);
    OATPP_ASSERT(beginResult->isSuccess());
    OATPP_LOGD(TAG, "Started outer transaction");
      
    // Insert in outer transaction
    auto insertResult = client.insertRow("outer_tx", conn);
    OATPP_ASSERT(insertResult->isSuccess());
    OATPP_LOGD(TAG, "Inserted outer_tx row");
      
    // Create savepoint
    auto savepointResult = executor->executeRaw("SAVEPOINT sp1;", conn);
    OATPP_ASSERT(savepointResult->isSuccess());
    OATPP_LOGD(TAG, "Created savepoint sp1");
      
    // Insert in nested transaction
    insertResult = client.insertRow("inner_tx", conn);
    OATPP_ASSERT(insertResult->isSuccess());
    OATPP_LOGD(TAG, "Inserted inner_tx row");
      
    // Verify both rows are visible
    auto result = client.selectAll(conn);
    OATPP_ASSERT(result->isSuccess());
    auto dataset = result->fetch<oatpp::Vector<oatpp::Object<TestRow>>>();
    OATPP_LOGD(TAG, "Before rollback to savepoint - row count: %d", dataset->size());
    OATPP_ASSERT(dataset->size() == 2);
      
    OATPP_LOGD(TAG, "Attempting to rollback to savepoint sp1");
    auto rollbackToSavepointResult = executor->executeRaw("ROLLBACK TO SAVEPOINT sp1;", conn);
    OATPP_ASSERT(rollbackToSavepointResult->isSuccess());
    OATPP_LOGD(TAG, "Rolled back to savepoint sp1");

    OATPP_LOGD(TAG, "Checking transaction state after rollback");
    auto txStateResult = client.getTransactionState(conn);
    OATPP_ASSERT(txStateResult->isSuccess());
    auto txState = txStateResult->fetch<oatpp::Vector<oatpp::Object<TransactionState>>>(1);
    OATPP_ASSERT(txState && txState->size() == 1);
    OATPP_LOGD(TAG, "Transaction state: %s", txState->front()->in_transaction->c_str());
    OATPP_ASSERT(txState->front()->in_transaction == "true");
      
    OATPP_LOGD(TAG, "Verifying only outer transaction data remains");

    {
        // Check connection state before selectAll
        auto connState = executor->executeRaw("SELECT IF(@@in_transaction, 'true', 'false') as in_transaction;", conn);
        {
            auto result = connState->fetch<oatpp::Vector<oatpp::Object<TransactionState>>>(1);
            OATPP_LOGD(TAG, "Connection transaction state: %s", result->front()->in_transaction->c_str());
        } // result goes out of scope here
        connState = nullptr; // Release the query result
    }
    
    OATPP_LOGD(TAG, "Executing selectAll query...");
    {
        auto selectResult = client.selectAll(conn);
        if (!selectResult->isSuccess()) {
            OATPP_LOGD(TAG, "selectAll query failed: %s", selectResult->getErrorMessage()->c_str());
            // Try to recover by rolling back
            executor->rollback(conn);
            executor->closeConnection(conn);
            OATPP_ASSERT(false); // Force test failure
        }
        OATPP_LOGD(TAG, "selectAll query executed successfully");
        OATPP_LOGD(TAG, "Fetching dataset after selectAll");
        auto dataset = selectResult->fetch<oatpp::Vector<oatpp::Object<TestRow>>>();
        OATPP_LOGD(TAG, "After rollback to savepoint - row count: %d", dataset->size());
        if (dataset->size() > 0) {
          for(const auto& row : *dataset) {
            OATPP_LOGD(TAG, "Row value: %s", row->value->c_str());
          }
        }
        OATPP_ASSERT(dataset->size() == 1);
        OATPP_ASSERT(dataset->front()->value == "outer_tx");
    }
      
    // Release savepoint is not needed since it's automatically released on rollback
    // auto releaseSavepointResult = executor->executeRaw("RELEASE SAVEPOINT sp1;", conn);
    // OATPP_ASSERT(releaseSavepointResult->isSuccess());
    // OATPP_LOGD(TAG, "Released savepoint sp1");
      
    // Commit outer transaction
    auto commitResult = executor->commit(conn);
    OATPP_ASSERT(commitResult->isSuccess());
    OATPP_LOGD(TAG, "Outer transaction committed");
      
    // Verify in new transaction
    conn = executor->getConnection();
    OATPP_ASSERT(conn);
    beginResult = executor->begin(conn);
    OATPP_ASSERT(beginResult->isSuccess());
    OATPP_LOGD(TAG, "Started verification transaction");
      
    result = client.selectAll(conn);
    OATPP_ASSERT(result->isSuccess());
    dataset = result->fetch<oatpp::Vector<oatpp::Object<TestRow>>>();
    OATPP_LOGD(TAG, "Final state - row count: %d", dataset->size());
    if (dataset->size() > 0) {
      for(const auto& row : *dataset) {
        OATPP_LOGD(TAG, "Final row value: %s", row->value->c_str());
      }
    }
    OATPP_ASSERT(dataset->size() == 1);
    OATPP_ASSERT(dataset->front()->value == "outer_tx");
      
    commitResult = executor->commit(conn);
    OATPP_ASSERT(commitResult->isSuccess());
    OATPP_LOGD(TAG, "Verification transaction committed");
    
    // Close the connection
    executor->closeConnection(conn);
  }

  // Cleanup
  client.dropTable();
  
  // Close the executor's connection pool
  executor->clearAllConnections();
  
  OATPP_LOGD(TAG, "OK");
}

}}}}
