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
  const char* error = nullptr;

  // Free the result set if any
  if (mysql_stmt_free_result(m_stmt)) {
    error = mysql_stmt_error(m_stmt);
    OATPP_LOGD("QueryResult", "Error freeing result set: %s", error);
    success = false;
  }

  // Close the statement
  if (mysql_stmt_close(m_stmt)) {
    error = mysql_stmt_error(m_stmt);
    OATPP_LOGD("QueryResult", "Error closing statement: %s", error);
    success = false;
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

oatpp::Void QueryResult::fetch(const oatpp::Type* const type, v_int64 count) {
  // OATPP_LOGD("QueryResult::fetch", "Fetching %d rows, type_id=%d, type_name=%s", count, type->classId.id, type->classId.name);
  return m_resultMapper->readRows(&m_resultData, type, count);
}

}}