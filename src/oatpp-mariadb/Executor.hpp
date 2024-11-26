#ifndef oatpp_mariadb_Executor_hpp
#define oatpp_mariadb_Executor_hpp

#include "oatpp/core/data/share/StringTemplate.hpp"
#include "ConnectionProvider.hpp"
#include "Connection.hpp"
#include "QueryResult.hpp"
#include "mapping/Serializer.hpp"
#include "ql_template/Parser.hpp"
#include "ql_template/TemplateValueProvider.hpp"
#include <functional>

#include "oatpp/orm/Executor.hpp"

namespace oatpp { namespace mariadb {

class Executor : public orm::Executor {
private:
  /*
   * We need this invalidator to correlate abstract orm::Connection to its correct invalidator.
   */
  class ConnectionInvalidator : public provider::Invalidator<orm::Connection> {
  public:
    void invalidate(const std::shared_ptr<orm::Connection>& connection) override;
  };

private:
  std::shared_ptr<ConnectionInvalidator> m_connectionInvalidator;
  std::shared_ptr<provider::Provider<Connection>> m_connectionProvider;
  std::shared_ptr<mapping::Serializer> m_serializer;
  std::shared_ptr<mapping::ResultMapper> m_resultMapper;
  std::shared_ptr<data::mapping::TypeResolver> m_defaultTypeResolver;

private:
  struct QueryParameter {
    oatpp::String name;
    std::vector<std::string> propertyPath;
  };

  QueryParameter parseQueryParameter(const oatpp::String& paramName);

private:
  void bindParams(MYSQL_STMT* stmt,
                  const StringTemplate& queryTemplate,
                  const std::unordered_map<oatpp::String, oatpp::Void>& params,
                  const std::shared_ptr<const data::mapping::TypeResolver>& typeResolver);

  bool isDeadlockError(const char* error) {
    return error && (strstr(error, "Deadlock") != nullptr || 
                    strstr(error, "Lock wait timeout") != nullptr);
  }

  std::string std_str(const oatpp::String& str) {
    if (!str) return "NULL";
    std::string result;
    result.reserve(str->length() * 2);  // Reserve space for potential escaping
    for (const char c : *str) {
      switch (c) {
        case '\'': result += "''"; break;
        case '\\': result += "\\\\"; break;
        case '\n': result += "\\n"; break;
        case '\r': result += "\\r"; break;
        case '\t': result += "\\t"; break;
        default: result += c;
      }
    }
    return result;
  }

  oatpp::String getSchemaVersionTableName(const oatpp::String& suffix) {
    auto tableName = oatpp::String("oatpp_schema_version");
    if (suffix && suffix->length() > 0) {
      tableName = tableName + "_" + suffix;
    }
    return tableName;
  }

  static constexpr v_int32 MAX_RETRIES = 3;
  static constexpr v_int32 MAX_SCRIPT_LENGTH = 1024 * 1024;  // 1MB
  static constexpr v_int64 MIN_VERSION = 0;
  static constexpr v_int64 MAX_VERSION = 9223372036854775807LL;  // Max BIGINT

  class MigrationError : public std::runtime_error {
  public:
    MigrationError(const std::string& message) : std::runtime_error(message) {}
  };

  class ConcurrencyError : public MigrationError {
  public:
    ConcurrencyError(const std::string& message) : MigrationError(message) {}
  };

private:
  void validateMigrationScript(const oatpp::String& script, v_int64 newVersion);
  void validateSchemaVersion(v_int64 currentVersion, v_int64 newVersion);

protected:
  std::shared_ptr<orm::QueryResult> execute(const oatpp::String& query,
                                          const std::shared_ptr<const data::mapping::TypeResolver>& typeResolver,
                                          const provider::ResourceHandle<orm::Connection>& connection);

  void rollbackToSavepoint(const provider::ResourceHandle<orm::Connection>& connection,
                          const String& savepointName);

  void setSavepoint(const provider::ResourceHandle<orm::Connection>& connection,
                   const String& savepointName);

  void releaseSavepoint(const provider::ResourceHandle<orm::Connection>& connection,
                       const String& savepointName);

public:

  Executor(const std::shared_ptr<provider::Provider<Connection>>& connectionProvider);

  /**
   * Get default type resolver.
   * @return
   */
  std::shared_ptr<data::mapping::TypeResolver> createTypeResolver() override;

  /**
   * Get database connection.
   * @return
   */
  provider::ResourceHandle<orm::Connection> getConnection() override;

  /**
   * Parse query template.
   * @param name - template name.
   * @param text - template text.
   * @param paramsTypeMap - template parameter types.
   * @param prepare - `true` if the query should use prepared statement, `false` otherwise.
   * @return - &id:oatpp::data::share::StringTemplate;.
   */
  StringTemplate parseQueryTemplate(const oatpp::String& name,
                                    const oatpp::String& text,
                                    const ParamsTypeMap& paramsTypeMap,
                                    bool prepare = false) override;

  /**
   * Execute database query using a query template.
   * @param queryTemplate - a query template obtained in a prior call to &l:Executor::parseQueryTemplate (); method.
   * @param params - query parameters.
   * @param enabledInterpretations - enabled type interpretations.
   * @param connection - database connection.
   * @return - &id:oatpp::orm::QueryResult;.
   */
  std::shared_ptr<orm::QueryResult> execute(const StringTemplate& queryTemplate,
                                            const std::unordered_map<oatpp::String, oatpp::Void>& params,
                                            const std::shared_ptr<const data::mapping::TypeResolver>& typeResolver = nullptr,
                                            const provider::ResourceHandle<orm::Connection>& connection = nullptr)  override;

  /**
   * Execute database query using a query template without parameters.
   * @param queryTemplate - a query template obtained in a prior call to &l:Executor::parseQueryTemplate (); method.
   * @param connection - database connection.
   * @return - &id:oatpp::orm::QueryResult;.
   */
  std::shared_ptr<orm::QueryResult> execute(const StringTemplate& queryTemplate,
                                          const provider::ResourceHandle<orm::Connection>& connection = nullptr);

  /**
   * Execute raw SQL query.
   * @param query - SQL query string.
   * @param connection - database connection.
   * @return - &id:oatpp::orm::QueryResult;.
   */
  std::shared_ptr<orm::QueryResult> executeRaw(const oatpp::String& query,
                                             const provider::ResourceHandle<orm::Connection>& connection = nullptr);

  /**
   * Begin database transaction. Should NOT be used directly. Use &id:oatpp::orm::Transaction; instead.
   * @param connection - database connection.
   * @return - &id:oatpp::orm::QueryResult;.
   */
  std::shared_ptr<orm::QueryResult> begin(const provider::ResourceHandle<orm::Connection>& connection) override;

  /**
   * Commit database transaction. Should NOT be used directly. Use &id:oatpp::orm::Transaction; instead.
   * @param connection
   * @return - &id:oatpp::orm::QueryResult;.
   */
  std::shared_ptr<orm::QueryResult> commit(const provider::ResourceHandle<orm::Connection>& connection) override;

  /**
   * Rollback database transaction. Should NOT be used directly. Use &id:oatpp::orm::Transaction; instead.
   * @param connection
   * @return - &id:oatpp::orm::QueryResult;.
   */
  std::shared_ptr<orm::QueryResult> rollback(const provider::ResourceHandle<orm::Connection>& connection) override;

  /**
   * Get current database schema version.
   * @param suffix - suffix or table name for schema version control.
   * @param connection - database connection.
   * @return - schema version.
   */
  v_int64 getSchemaVersion(const oatpp::String& suffix = nullptr,
                          const provider::ResourceHandle<orm::Connection>& connection = nullptr) override;

  /**
   * Run schema migration script.
   * @param script - script text.
   * @param newVersion - schema version corresponding to this script.
   * @param suffix - suffix or table name for schema version control.
   * @param connection - database connection.
   */
  void migrateSchema(const oatpp::String& script,
                    v_int64 newVersion,
                    const oatpp::String& suffix = nullptr,
                    const provider::ResourceHandle<orm::Connection>& connection = nullptr) override;

  /**
   * Close a specific database connection.
   * @param connection - database connection to close
   */
  void closeConnection(const provider::ResourceHandle<orm::Connection>& connection);

  /**
   * Clear and close all connections in the pool.
   */
  void clearAllConnections();

  /**
   * Retry an operation if it encounters a deadlock
   */
  void retryOnDeadlock(const std::function<void()>& operation);

  /**
   * Acquire migration lock
   */
  void acquireMigrationLock(const provider::ResourceHandle<orm::Connection>& connection,
                          const oatpp::String& tableName,
                          v_uint32 timeoutSeconds = 10);

  /**
   * Release migration lock
   */
  void releaseMigrationLock(const provider::ResourceHandle<orm::Connection>& connection);

  /**
   * Log migration error
   */
  void logMigrationError(const provider::ResourceHandle<orm::Connection>& connection,
                        const oatpp::String& tableName,
                        v_int64 version,
                        const std::string& error);

};

}}

#endif /* oatpp_mariadb_Executor_hpp */