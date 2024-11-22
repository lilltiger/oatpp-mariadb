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

QueryResult::~QueryResult() {
  if (m_stmt) {
    mysql_stmt_close(m_stmt);
    OATPP_LOGD("QueryResult", "Statement closed and QueryResult destroyed");
  } else {
    OATPP_LOGD("QueryResult", "No statement to close, QueryResult destroyed");
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