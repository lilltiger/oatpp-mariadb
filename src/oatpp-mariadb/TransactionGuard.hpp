#ifndef oatpp_mariadb_TransactionGuard_hpp
#define oatpp_mariadb_TransactionGuard_hpp

#include "Executor.hpp"
#include "oatpp/core/base/Environment.hpp"
#include <memory>
#include <functional>
#include <chrono>
#include <thread>
#include <string>

namespace oatpp { namespace mariadb {

/**
 * A RAII-style transaction guard that handles deadlocks and retries
 */
class TransactionGuard {
private:
  std::shared_ptr<orm::Executor> m_executor;
  provider::ResourceHandle<orm::Connection> m_connection;
  v_int32 m_maxRetries;
  bool m_committed;
  
public:
  /**
   * Constructor.
   * @param executor - Database executor.
   * @param maxRetries - Maximum number of retries in case of deadlock.
   */
  TransactionGuard(const std::shared_ptr<orm::Executor>& executor, v_int32 maxRetries)
    : m_executor(executor)
    , m_maxRetries(maxRetries)
    , m_committed(false)
  {
    m_connection = m_executor->getConnection();
    auto result = m_executor->begin(m_connection);
    if (!result->isSuccess()) {
      std::string errorMsg = "Failed to begin transaction: ";
      errorMsg += result->getErrorMessage()->c_str();
      throw std::runtime_error(errorMsg);
    }
  }
  
  /**
   * Destructor. Rolls back the transaction if not committed.
   */
  ~TransactionGuard() {
    if (!m_committed && m_connection) {
      m_executor->rollback(m_connection);
    }
  }
  
  /**
   * Get the connection associated with this transaction.
   * @return Connection handle.
   */
  provider::ResourceHandle<orm::Connection>& getConnection() {
    return m_connection;
  }
  
  /**
   * Execute a transaction with automatic retry on deadlock.
   * @param action - Lambda containing the transaction logic.
   * @return true if transaction succeeded, false otherwise.
   */
  bool execute(const std::function<bool(const provider::ResourceHandle<orm::Connection>&)>& action) {
    v_int32 retries = 0;
    
    while (retries < m_maxRetries) {
      try {
        if (action(m_connection)) {
          auto result = m_executor->commit(m_connection);
          if (result->isSuccess()) {
            m_committed = true;
            return true;
          }
          
          std::string errorMsg = result->getErrorMessage()->c_str();
          if (errorMsg.find("deadlock") == std::string::npos) {
            return false;
          }
        }
      } catch (const std::exception& e) {
        OATPP_LOGD("TransactionGuard", "Transaction failed: %s", e.what());
      }
      
      // Rollback and retry
      m_executor->rollback(m_connection);
      retries++;
      
      if (retries < m_maxRetries) {
        // Exponential backoff
        std::this_thread::sleep_for(std::chrono::milliseconds(100 * (1 << retries)));
        
        // Get a fresh connection for retry
        m_connection = m_executor->getConnection();
        auto result = m_executor->begin(m_connection);
        if (!result->isSuccess()) {
          return false;
        }
      }
    }
    
    return false;
  }
  
  /**
   * Commit the transaction.
   * @return true if commit succeeded, false otherwise.
   */
  bool commit() {
    if (!m_committed) {
      auto result = m_executor->commit(m_connection);
      if (result->isSuccess()) {
        m_committed = true;
        return true;
      }
    }
    return false;
  }
  
  /**
   * Rollback the transaction.
   */
  void rollback() {
    if (!m_committed) {
      m_executor->rollback(m_connection);
    }
  }
};

}}

#endif /* oatpp_mariadb_TransactionGuard_hpp */
