#include "QueryResult.hpp"

namespace oatpp { namespace mariadb {

QueryResult::QueryResult(MYSQL_STMT* stmt,
                         const provider::ResourceHandle<orm::Connection>& connection,
                         const std::shared_ptr<mapping::ResultMapper>& resultMapper,
                         const std::shared_ptr<const data::mapping::TypeResolver>& typeResolver)
  : m_stmt(stmt)
  , m_connection(connection)
  , m_resultMapper(resultMapper)
  , m_resultData(stmt, typeResolver)
  , m_inTransaction(false)
  , m_lastInsertId(-1)
  , m_hasBeenFetched(false)
  , m_cachingEnabled(false)
  , m_cachedResult(nullptr)
{
  OATPP_LOGD("QueryResult", "Executing statement...");
  
  if (!m_stmt) {
    m_errorMessage = "Statement is null";
    OATPP_LOGD("QueryResult", "Error: Statement is null");
    return;
  }

  MYSQL* mysql = std::static_pointer_cast<mariadb::Connection>(m_connection.object)->getHandle();
  if (!mysql) {
    m_errorMessage = "MySQL connection handle is null";
    OATPP_LOGD("QueryResult", "Error: MySQL connection handle is null");
    return;
  }

  // Check if we're in a transaction
  if (mysql_query(mysql, "SELECT IF(@@in_transaction, 'true', 'false') as in_transaction") == 0) {
    MYSQL_RES* res = mysql_store_result(mysql);
    if (res) {
      MYSQL_ROW row = mysql_fetch_row(res);
      if (row && row[0]) {
        m_inTransaction = (strcmp(row[0], "true") == 0);
      }
      mysql_free_result(res);
    }
  }

  OATPP_LOGD("QueryResult", "MySQL thread id: %lu", mysql_thread_id(mysql));
  OATPP_LOGD("QueryResult", "Statement address: %p", (void*)m_stmt);
  
  if (mysql_stmt_execute(m_stmt)) {
    m_errorMessage = "Error executing statement: " + std::string(mysql_stmt_error(m_stmt));
    OATPP_LOGD("QueryResult", "Statement execution error: %s", m_errorMessage->c_str());
    OATPP_LOGD("QueryResult", "MySQL error: %s", mysql_error(mysql));
    return;
  }

  OATPP_LOGD("QueryResult", "Statement executed successfully");
  m_resultData.init();    // initialize the information of all columns
  OATPP_LOGD("QueryResult", "Result data initialized");
}

bool QueryResult::cleanupStatement() {
  if (!m_stmt) {
    return true;
  }

  bool success = true;

  // Get the MySQL connection handle
  MYSQL* mysql = m_connection ? std::static_pointer_cast<mariadb::Connection>(m_connection.object)->getHandle() : nullptr;
  
  // Only try to free the result set if we have a valid connection and metadata
  if (mysql && !mysql_ping(mysql)) {  // Check if connection is alive
    MYSQL_RES* metadata = mysql_stmt_result_metadata(m_stmt);
    if (metadata) {
      mysql_free_result(metadata);
      if (mysql_stmt_free_result(m_stmt)) {
        OATPP_LOGD("QueryResult", "Error freeing result set: %s", mysql_stmt_error(m_stmt));
        success = false;
      }
    }

    // Close the statement
    if (mysql_stmt_close(m_stmt)) {
      OATPP_LOGD("QueryResult", "Error closing statement: %s", mysql_stmt_error(m_stmt));
      success = false;
    }
  } else {
    // Connection is lost, just mark the statement as cleaned up
    OATPP_LOGD("QueryResult", "Connection lost, skipping result set cleanup");
    success = true;  // Consider it successful since we can't do anything about it
  }

  m_stmt = nullptr;
  return success;
}

QueryResult::~QueryResult() {
  if (cleanupStatement()) {
    OATPP_LOGD("QueryResult", "Statement cleaned up successfully");
  } else {
    OATPP_LOGD("QueryResult", "Statement cleanup failed");
  }
}

provider::ResourceHandle<orm::Connection> QueryResult::getConnection() const {
  return m_connection;
}

bool QueryResult::isSuccess() const {
  return m_resultData.isSuccess;
}

oatpp::String QueryResult::getErrorMessage() const {
  return m_errorMessage;
}

v_int64 QueryResult::getPosition() const {
  return m_resultData.rowIndex;
}

v_int64 QueryResult::getKnownCount() const {
  return -1;
}

bool QueryResult::hasMoreToFetch() const {
  return m_resultData.hasMore;
}

bool QueryResult::hasBeenFetched() const {
  return m_hasBeenFetched;
}

void QueryResult::enableResultCaching(bool enable) {
  m_cachingEnabled = enable;
  if (!enable) {
    m_cachedResult = nullptr;
  }
}

bool QueryResult::isResultCachingEnabled() const {
  return m_cachingEnabled;
}

oatpp::Void QueryResult::fetch(const oatpp::Type* const type, v_int64 count) {
  if (m_hasBeenFetched) {
    OATPP_LOGW("QueryResult", "Warning: Attempting to fetch results multiple times.");
    if (m_cachingEnabled && m_cachedResult) {
      OATPP_LOGD("QueryResult", "Returning cached results");
      return m_cachedResult;
    }
    return nullptr;
  }
  
  m_hasBeenFetched = true;
  auto result = m_resultMapper->readRows(&m_resultData, type, count);
  
  if (m_cachingEnabled) {
    OATPP_LOGD("QueryResult", "Caching query results");
    m_cachedResult = result;
  }
  
  return result;
}

v_int64 QueryResult::getLastInsertId() const {
  if (m_lastInsertId >= 0) {
    // Return ID from RETURNING clause if available
    return m_lastInsertId;
  }
  
  if (!m_stmt) {
    return 0;
  }
  MYSQL* mysql = std::static_pointer_cast<mariadb::Connection>(m_connection.object)->getHandle();
  if (!mysql) {
    return 0;
  }
  return mysql_insert_id(mysql);
}

void QueryResult::setLastInsertId(v_int64 id) {
  m_lastInsertId = id;
}

v_int64 QueryResult::getAffectedRows() const {
  if (!m_stmt) {
    return 0;
  }
  return mysql_stmt_affected_rows(m_stmt);
}

}}