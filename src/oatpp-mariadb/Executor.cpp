#include "Executor.hpp"
#include "Connection.hpp"
#include "dto/CountResult.hpp"

#include "QueryResult.hpp"
#include "mapping/ResultMapper.hpp"
#include "oatpp/core/base/Environment.hpp"
#include "oatpp/core/data/stream/Stream.hpp"
#include "oatpp/core/utils/ConversionUtils.hpp"
#include "oatpp/core/data/mapping/type/Type.hpp"
#include "oatpp/core/Types.hpp"

namespace oatpp { namespace mariadb {

#include OATPP_CODEGEN_BEGIN(DTO)

namespace {

class CountResult : public oatpp::DTO {
  DTO_INIT(CountResult, DTO);
  DTO_FIELD(Int64, total, "total") = nullptr;
};

class VersionResult : public oatpp::DTO {
  DTO_INIT(VersionResult, DTO);
  DTO_FIELD(Int64, version, "version") = nullptr;
};

class LockResult : public oatpp::DTO {
  DTO_INIT(LockResult, DTO);
  DTO_FIELD(Int64, lock_status, "lock_status") = nullptr;
};

}

void Executor::ConnectionInvalidator::invalidate(const std::shared_ptr<orm::Connection>& connection) {
  auto c = std::static_pointer_cast<Connection>(connection);
  auto invalidator = c->getInvalidator();
  if(!invalidator) {
    throw std::runtime_error("[oatpp::mariadb::Executor::ConnectionInvalidator::invalidate()]: Error. "
                             "Connection invalidator was NOT set.");
  }
  invalidator->invalidate(c);
}

Executor::Executor(const std::shared_ptr<provider::Provider<Connection>>& connectionProvider)
  : m_connectionProvider(connectionProvider)
  , m_connectionInvalidator(std::make_shared<ConnectionInvalidator>())
  , m_serializer(std::make_shared<mapping::Serializer>())
  , m_resultMapper(std::make_shared<mapping::ResultMapper>())
  , m_defaultTypeResolver(createTypeResolver())
{

}

std::shared_ptr<data::mapping::TypeResolver> Executor::createTypeResolver() {
  auto resolver = std::make_shared<data::mapping::TypeResolver>();
  return resolver;
}

provider::ResourceHandle<orm::Connection> Executor::getConnection() {
  auto connection = m_connectionProvider->get();
  if (connection) {
    connection.object->setInvalidator(connection.invalidator);
    return provider::ResourceHandle<orm::Connection>(
      connection.object,
      m_connectionInvalidator
    );
  }
  throw std::runtime_error("[oatpp::mariadb::Executor::getConnection()]: Error. Can't connect.");
}

data::share::StringTemplate Executor::parseQueryTemplate(const oatpp::String& name,
                                                         const oatpp::String& text,
                                                         const ParamsTypeMap& paramsTypeMap,
                                                         bool prepare) {
  (void) paramsTypeMap;

  auto&& t = ql_template::Parser::parseTemplate(text);

  auto extra = std::make_shared<ql_template::Parser::TemplateExtra>();
  t.setExtraData(extra);

  extra->prepare = prepare;
  extra->templateName = name;

  ql_template::TemplateValueProvider valueProvider;
  extra->preparedTemplate = t.format(&valueProvider);

  return t;
}

// e.g. "user.name.first" -> QueryParameter{name="user", propertyPath={"name", "first"}}
Executor::QueryParameter Executor::parseQueryParameter(const oatpp::String& paramName) {

  parser::Caret caret(paramName);
  auto nameLabel = caret.putLabel();
  if(caret.findChar('.') && caret.getPosition() < caret.getDataSize() - 1) {

    QueryParameter result;
    result.name = nameLabel.toString();

    do {

      caret.inc();
      auto label = caret.putLabel();
      caret.findChar('.');
      result.propertyPath.push_back(label.std_str());

    } while (caret.getPosition() < caret.getDataSize());

    return result;

  }

  return {nameLabel.toString(), {}};

}

// mysql bind params
void Executor::bindParams(MYSQL_STMT* stmt,
                          const StringTemplate& queryTemplate,
                          const std::unordered_map<oatpp::String, oatpp::Void>& params, 
                          const std::shared_ptr<const data::mapping::TypeResolver>& typeResolver) {
  data::mapping::TypeResolver::Cache cache;

  auto extra = std::static_pointer_cast<ql_template::Parser::TemplateExtra>(queryTemplate.getExtraData());

  size_t count = queryTemplate.getTemplateVariables().size();
  for (size_t i = 0; i < count; ++i) {
    auto& var = queryTemplate.getTemplateVariables()[i];
    
    auto queryParam = parseQueryParameter(var.name);  // e.g. "user.name.first" -> QueryParameter{name="user", propertyPath={"name", "first"}}

    if (queryParam.name->empty()) {
      throw std::runtime_error(std::string("[oatpp::mariadb::Executor::bindParams()]: Error. Can't parse query parameter name. Parameter name: ") + 
                             var.name->c_str());
    }

    // resolve parameter type
    auto it = params.find(queryParam.name);
    if (it != params.end()) {
      auto value = typeResolver->resolveObjectPropertyValue(it->second, queryParam.propertyPath, cache);
      if (value.getValueType()->classId.id == oatpp::Void::Class::CLASS_ID.id) {
        throw std::runtime_error(std::string("[oatpp::mariadb::Executor::bindParams()]: Error. Can't resolve parameter type because property dose not found or its type is unknown. Parameter name: ") + 
                               queryParam.name->c_str() + ", var.name: " + var.name->c_str());
      }

      // [serialize] bind parameter according to the resolved type
      m_serializer->serialize(stmt, i, value);
    }
  }

  // Bind all parameters at once after serialization is complete
  m_serializer->bindParameters(stmt);
}

std::shared_ptr<orm::QueryResult> Executor::execute(const StringTemplate& queryTemplate,
                                                    const std::unordered_map<oatpp::String, oatpp::Void>& params,
                                                    const std::shared_ptr<const data::mapping::TypeResolver>& typeResolver,
                                                    const provider::ResourceHandle<orm::Connection>& connection) {

  auto connectionHandle = connection;
  if (!connectionHandle) {
    connectionHandle = getConnection();
  }

  std::shared_ptr<const data::mapping::TypeResolver> tr = typeResolver;
  if(!tr) {
    tr = m_defaultTypeResolver;
  }

  auto mysqlConnection = std::static_pointer_cast<mariadb::Connection>(connectionHandle.object);

  auto extra = std::static_pointer_cast<ql_template::Parser::TemplateExtra>(queryTemplate.getExtraData());
  auto conn = std::static_pointer_cast<mariadb::Connection>(connectionHandle.object)->getHandle();

  OATPP_LOGD("Executor", "Preparing to execute query. Connection thread id: %lu", mysql_thread_id(conn));
  OATPP_LOGD("Executor", "Query template: %s", extra->preparedTemplate->c_str());

  MYSQL_STMT* stmt = mysql_stmt_init(conn);
  if (!stmt) {
    throw std::runtime_error(std::string("[oatpp::mariadb::Executor::execute()]: Error. Unable to initialize statement."));
  }

  OATPP_LOGD("Executor", "Statement initialized. Address: %p", (void*)stmt);

  if (mysql_stmt_prepare(stmt, extra->preparedTemplate->c_str(), extra->preparedTemplate->size())) {
    std::string error = mysql_stmt_error(stmt);
    mysql_stmt_close(stmt);
    throw std::runtime_error(std::string("[oatpp::mariadb::Executor::execute()]: Error. Unable to prepare statement: ") + error);
  }

  OATPP_LOGD("Executor", "Statement prepared successfully");

  if (!params.empty()) {
    OATPP_LOGD("Executor", "Binding parameters...");
    bindParams(stmt, queryTemplate, params, typeResolver);
    OATPP_LOGD("Executor", "Parameters bound successfully");
  }

  return std::make_shared<QueryResult>(stmt, connectionHandle, m_resultMapper, tr);
}

std::shared_ptr<orm::QueryResult> Executor::execute(const StringTemplate& queryTemplate,
                                                  const provider::ResourceHandle<orm::Connection>& connection) {
  return execute(queryTemplate, {}, m_defaultTypeResolver, connection);
}

std::shared_ptr<orm::QueryResult> Executor::execute(const oatpp::String& query,
                                             const std::shared_ptr<const data::mapping::TypeResolver>& typeResolver,
                                             const provider::ResourceHandle<orm::Connection>& connection) {
  
  auto mysqlConnection = std::static_pointer_cast<mariadb::Connection>(connection.object);
  if (!mysqlConnection) {
    throw std::runtime_error(std::string("[oatpp::mariadb::Executor::execute]: Invalid connection"));
  }

  MYSQL* mysql = mysqlConnection->getHandle();
  if (!mysql) {
    throw std::runtime_error(std::string("[oatpp::mariadb::Executor::execute]: MySQL connection handle is null"));
  }

  MYSQL_STMT* stmt = mysql_stmt_init(mysql);
  if (!stmt) {
    throw std::runtime_error(std::string("[oatpp::mariadb::Executor::execute]: Failed to initialize statement"));
  }

  if (mysql_stmt_prepare(stmt, query->c_str(), query->size())) {
    auto error = std::string(mysql_stmt_error(stmt));
    mysql_stmt_close(stmt);
    throw std::runtime_error(std::string("[oatpp::mariadb::Executor::execute]: Statement prepare error: ") + error);
  }

  auto resultMapper = std::make_shared<mapping::ResultMapper>();
  return std::make_shared<QueryResult>(stmt, connection, resultMapper, typeResolver);
}

std::shared_ptr<orm::QueryResult> Executor::executeRaw(const oatpp::String& query,
                                                     const provider::ResourceHandle<orm::Connection>& connection) {
  auto connectionHandle = connection;
  if (!connectionHandle) {
    connectionHandle = getConnection();
  }

  auto mysqlConnection = std::static_pointer_cast<mariadb::Connection>(connectionHandle.object);
  
  MYSQL_STMT* stmt = mysql_stmt_init(mysqlConnection->getHandle());
  if (!stmt) {
    throw std::runtime_error(std::string("[oatpp::mariadb::Executor::executeRaw()]: Error. Can't create MYSQL_STMT. Error: ") + 
                           mysql_error(mysqlConnection->getHandle()));
  }

  if (mysql_stmt_prepare(stmt, query->c_str(), query->size())) {
    throw std::runtime_error(std::string("[oatpp::mariadb::Executor::executeRaw()]: Error. Can't prepare MYSQL_STMT. Query: ") + 
                           query->c_str() + " Error: " + mysql_stmt_error(stmt));
  }

  if (mysql_stmt_execute(stmt)) {
    throw std::runtime_error(std::string("[oatpp::mariadb::Executor::executeRaw()]: Error. Can't execute MYSQL_STMT. Query: ") + 
                           query->c_str() + " Error: " + mysql_stmt_error(stmt));
  }

  return std::make_shared<mariadb::QueryResult>(stmt, connectionHandle, m_resultMapper, nullptr);
}

std::shared_ptr<orm::QueryResult> Executor::begin(const provider::ResourceHandle<orm::Connection>& connection) {
  return execute("START TRANSACTION;", nullptr, connection);
}

std::shared_ptr<orm::QueryResult> Executor::commit(const provider::ResourceHandle<orm::Connection>& connection) {
  return execute("COMMIT;", nullptr, connection);
}

std::shared_ptr<orm::QueryResult> Executor::rollback(const provider::ResourceHandle<orm::Connection>& connection) {
  return execute("ROLLBACK;", nullptr, connection);
}

void Executor::rollbackToSavepoint(const provider::ResourceHandle<orm::Connection>& connection, const String& savepointName) {
  auto query = String("ROLLBACK TO SAVEPOINT ") + savepointName + ";";
  execute(query, nullptr, connection);
}

void Executor::setSavepoint(const provider::ResourceHandle<orm::Connection>& connection, const String& savepointName) {
  auto query = String("SAVEPOINT ") + savepointName + ";";
  execute(query, nullptr, connection);
}

void Executor::releaseSavepoint(const provider::ResourceHandle<orm::Connection>& connection, const String& savepointName) {
  auto query = String("RELEASE SAVEPOINT ") + savepointName + ";";
  execute(query, nullptr, connection);
}

v_int64 Executor::getSchemaVersion(const oatpp::String& suffix,
                                  const provider::ResourceHandle<orm::Connection>& connection) {
  auto tableName = getSchemaVersionTableName(suffix);
  auto conn = connection ? connection : getConnection();
  
  // First try to create the table if it doesn't exist
  auto createTableQuery = oatpp::String("CREATE TABLE IF NOT EXISTS `") + tableName + "` (version BIGINT NOT NULL PRIMARY KEY, timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP, INDEX idx_timestamp(timestamp)) ENGINE=InnoDB";
  auto createHistoryTableQuery = oatpp::String("CREATE TABLE IF NOT EXISTS `") + tableName + "_history` (id BIGINT AUTO_INCREMENT PRIMARY KEY, version BIGINT NOT NULL, script TEXT NOT NULL, status ENUM('pending', 'success', 'failed') NOT NULL, error TEXT, created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP, completed_at TIMESTAMP NULL, INDEX idx_version(version), INDEX idx_status(status), INDEX idx_created_at(created_at)) ENGINE=InnoDB";
    
  auto createResult = executeRaw(createTableQuery, conn);
  if (!createResult->isSuccess()) {
    throw std::runtime_error(std::string("[getSchemaVersion]: Failed to create version table: ") + 
                           createResult->getErrorMessage());
  }
  
  auto createHistoryResult = executeRaw(createHistoryTableQuery, conn);
  if (!createHistoryResult->isSuccess()) {
    throw std::runtime_error(std::string("[getSchemaVersion]: Failed to create history table: ") + 
                           createHistoryResult->getErrorMessage());
  }

  // Now proceed with locking and getting version
  auto lockQuery = oatpp::String("LOCK TABLES `") + tableName + "` WRITE, `" + tableName + "_history` WRITE";
  auto lockResult = executeRaw(lockQuery, conn);
  if (!lockResult->isSuccess()) {
    throw std::runtime_error(std::string("[getSchemaVersion]: Failed to lock tables: ") + 
                           lockResult->getErrorMessage());
  }

  try {
    // Check if we need to initialize with version 0
    auto checkQuery = oatpp::String("SELECT COUNT(*) as total FROM `") + tableName + "`";
    OATPP_LOGD("getSchemaVersion", "Executing query: %s", checkQuery->c_str());
    auto checkResult = executeRaw(checkQuery, conn);
    if (!checkResult || !checkResult->isSuccess()) {
      executeRaw("UNLOCK TABLES;", conn);
      OATPP_LOGE("getSchemaVersion", "Failed to execute check query");
      throw std::runtime_error("[getSchemaVersion]: Failed to execute check query");
    }

    auto countRow = checkResult->fetch<oatpp::Object<CountResult>>();
    if (!countRow || !countRow->total) {
      executeRaw("UNLOCK TABLES;", conn);
      OATPP_LOGE("getSchemaVersion", "Count row or total is null");
      throw std::runtime_error("[getSchemaVersion]: Count row or total is null");
    }

    OATPP_LOGD("getSchemaVersion", "Count value: %lld", static_cast<long long>(*countRow->total));
    if (*countRow->total == 0) {
      // Try to insert version 0, but handle the case where another process might have already done it
      try {
        auto insertQuery = oatpp::String("INSERT INTO `") + tableName + "` (version) VALUES (0)";
        auto insertResult = executeRaw(insertQuery, conn);
        if (!insertResult->isSuccess()) {
          OATPP_LOGD("getSchemaVersion", "Version 0 was already inserted by another process");
        }
      } catch (const std::exception& e) {
        OATPP_LOGD("getSchemaVersion", "Version 0 was already inserted by another process");
      }
    }

    // Get the current version
    OATPP_LOGD("getSchemaVersion", "Executing query: SELECT version FROM `%s` LIMIT 1", tableName->c_str());
    auto versionResult = executeRaw(oatpp::String("SELECT version FROM `") + tableName + "` LIMIT 1", conn);
    if (!versionResult || !versionResult->isSuccess()) {
      executeRaw("UNLOCK TABLES;", conn);
      OATPP_LOGE("getSchemaVersion", "Failed to get version");
      throw std::runtime_error("[getSchemaVersion]: Failed to get version");
    }

    auto versionRow = versionResult->fetch<oatpp::Object<VersionResult>>();
    if (!versionRow || !versionRow->version) {
      executeRaw("UNLOCK TABLES;", conn);
      OATPP_LOGE("getSchemaVersion", "No version rows found or version is null");
      throw std::runtime_error("[getSchemaVersion]: No version rows found or version is null");
    }

    auto version = *versionRow->version;
    executeRaw("UNLOCK TABLES;", conn);
    return version;

  } catch (...) {
    executeRaw("UNLOCK TABLES;", conn);
    throw;
  }
}

void Executor::migrateSchema(const oatpp::String& script,
                           v_int64 newVersion,
                           const oatpp::String& suffix,
                           const provider::ResourceHandle<orm::Connection>& connection) {

  auto tableName = getSchemaVersionTableName(suffix);
  
  OATPP_LOGD("migrateSchema", "Starting migration to version %lld", newVersion);
  
  // Set isolation level
  auto setIsolationResult = executeRaw("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE;", connection);
  if (!setIsolationResult->isSuccess()) {
    OATPP_LOGE("migrateSchema", "Failed to set isolation level: %s", setIsolationResult->getErrorMessage()->c_str());
    throw std::runtime_error("Failed to set isolation level: " + std::string(setIsolationResult->getErrorMessage()));
  }

  // Start transaction
  auto beginResult = begin(connection);
  if (!beginResult->isSuccess()) {
    OATPP_LOGE("migrateSchema", "Failed to begin transaction: %s", beginResult->getErrorMessage()->c_str());
    throw std::runtime_error("Failed to begin transaction: " + std::string(beginResult->getErrorMessage()));
  }

  try {
    // Lock tables
    auto lockQuery = oatpp::String("LOCK TABLES `") + tableName + "` WRITE, `" + tableName + "_history` WRITE";
    auto lockResult = executeRaw(lockQuery, connection);
    if (!lockResult->isSuccess()) {
      OATPP_LOGE("migrateSchema", "Failed to lock tables: %s", lockResult->getErrorMessage()->c_str());
      rollback(connection);
      throw std::runtime_error("Failed to lock tables: " + std::string(lockResult->getErrorMessage()));
    }

    // Check current version
    auto currentVersion = getSchemaVersion(suffix, connection);
    OATPP_LOGD("migrateSchema", "Current version: %lld, New version: %lld", currentVersion, newVersion);
    
    if (newVersion <= currentVersion) {
      OATPP_LOGE("migrateSchema", "New version (%lld) must be greater than current version (%lld)", newVersion, currentVersion);
      rollback(connection);
      executeRaw("UNLOCK TABLES;", connection);
      throw std::runtime_error("New version must be greater than current version");
    }

    // Log migration start
    auto insertHistoryText = oatpp::String("INSERT INTO `") + tableName + 
                           "_history` (version, script, status) VALUES (?, ?, ?)";
    auto insertHistoryQuery = parseQueryTemplate("insert_history", insertHistoryText, {
      {"1", oatpp::Int64::Class::getType()},
      {"2", oatpp::String::Class::getType()},
      {"3", oatpp::String::Class::getType()}
    }, true);
    
    std::unordered_map<oatpp::String, oatpp::Void> params;
    params["1"] = oatpp::Int64(newVersion);
    params["2"] = script;
    params["3"] = oatpp::String("pending");
    
    auto insertHistoryResult = execute(insertHistoryQuery, params, m_defaultTypeResolver, connection);
    if (!insertHistoryResult->isSuccess()) {
      OATPP_LOGE("migrateSchema", "Failed to log migration start: %s", insertHistoryResult->getErrorMessage()->c_str());
      rollback(connection);
      executeRaw("UNLOCK TABLES;", connection);
      throw std::runtime_error("Failed to log migration start: " + std::string(insertHistoryResult->getErrorMessage()));
    }

    // Execute migration script
    OATPP_LOGD("migrateSchema", "Executing migration script for version %lld", newVersion);
    auto scriptResult = executeRaw(script, connection);
    if (!scriptResult->isSuccess()) {
      auto errorMsg = scriptResult->getErrorMessage();
      OATPP_LOGE("migrateSchema", "Migration script failed: %s", errorMsg->c_str());
      
      // Log failure
      auto updateHistoryText = oatpp::String("UPDATE `") + tableName + "_history` SET status = ?, error = ?, completed_at = CURRENT_TIMESTAMP WHERE version = ? AND status = 'pending'";
      auto updateHistoryQuery = parseQueryTemplate("update_history_failed", updateHistoryText, {}, true);
      
      std::unordered_map<oatpp::String, oatpp::Void> errorParams;
      errorParams["1"] = oatpp::String("failed");
      errorParams["2"] = oatpp::String(errorMsg);
      errorParams["3"] = oatpp::Int64(newVersion);
      
      auto updateHistoryResult = execute(updateHistoryQuery, errorParams, m_defaultTypeResolver, connection);
      if (!updateHistoryResult->isSuccess()) {
        OATPP_LOGE("migrateSchema", "Failed to log migration failure: %s", updateHistoryResult->getErrorMessage()->c_str());
      }
      rollback(connection);
      executeRaw("UNLOCK TABLES;", connection);
      throw std::runtime_error("Migration script failed: " + std::string(errorMsg));
    }

    // Update schema version
    auto updateVersionText = oatpp::String("UPDATE `" + tableName + "` SET version = ?");
    auto updateVersionQuery = parseQueryTemplate("update_version", updateVersionText, {
      {"1", oatpp::Int64::Class::getType()}
    }, true);
    
    std::unordered_map<oatpp::String, oatpp::Void> versionParams;
    versionParams["1"] = oatpp::Int64(newVersion);
    
    auto updateResult = execute(updateVersionQuery, versionParams, m_defaultTypeResolver, connection);
    if (!updateResult->isSuccess()) {
      OATPP_LOGE("migrateSchema", "Failed to update version: %s", updateResult->getErrorMessage()->c_str());
      rollback(connection);
      executeRaw("UNLOCK TABLES;", connection);
      throw std::runtime_error("Failed to update version: " + std::string(updateResult->getErrorMessage()));
    }

    // Log success
    auto successHistoryText = oatpp::String("UPDATE `") + tableName + "_history` SET status = ?, completed_at = CURRENT_TIMESTAMP WHERE version = ? AND status = 'pending'";
    auto successHistoryQuery = parseQueryTemplate("success_history", successHistoryText, {}, true);
    
    std::unordered_map<oatpp::String, oatpp::Void> successParams;
    successParams["1"] = oatpp::String("success");
    successParams["2"] = oatpp::Int64(newVersion);
    auto successResult = execute(successHistoryQuery, successParams, m_defaultTypeResolver, connection);
    
    if (!successResult->isSuccess()) {
      OATPP_LOGE("migrateSchema", "Failed to log success: %s", successResult->getErrorMessage()->c_str());
    }

    // Commit transaction
    auto commitResult = commit(connection);
    if (!commitResult->isSuccess()) {
      OATPP_LOGE("migrateSchema", "Failed to commit transaction: %s", commitResult->getErrorMessage()->c_str());
      rollback(connection);
      executeRaw("UNLOCK TABLES;", connection);
      throw std::runtime_error("Failed to commit transaction: " + std::string(commitResult->getErrorMessage()));
    }

    executeRaw("UNLOCK TABLES;", connection);
    OATPP_LOGD("migrateSchema", "Successfully migrated to version %lld", newVersion);

  } catch (const std::exception& e) {
    OATPP_LOGE("migrateSchema", "Migration failed: %s", e.what());
    rollback(connection);
    executeRaw("UNLOCK TABLES;", connection);
    throw std::runtime_error(std::string("[migrateSchema]: ") + e.what());
  }
}

void Executor::validateMigrationScript(const oatpp::String& script, v_int64 newVersion) {
  if (!script) {
    throw MigrationError("Migration script cannot be null");
  }
  
  if (script->length() > MAX_SCRIPT_LENGTH) {
    throw MigrationError("Migration script exceeds maximum length of " + 
                        std::to_string(MAX_SCRIPT_LENGTH) + " bytes");
  }

  if (newVersion <= MIN_VERSION || newVersion > MAX_VERSION) {
    throw MigrationError("Invalid version number. Must be between " + 
                        std::to_string(MIN_VERSION + 1) + " and " + 
                        std::to_string(MAX_VERSION));
  }

  // Basic SQL injection prevention
  std::string scriptStr = std_str(script);
  std::vector<std::string> dangerousPatterns = {
    "DROP DATABASE",
    "DROP SCHEMA",
    "TRUNCATE DATABASE",
    "TRUNCATE SCHEMA"
  };

  for (const auto& pattern : dangerousPatterns) {
    if (scriptStr.find(pattern) != std::string::npos) {
      throw MigrationError("Migration script contains dangerous pattern: " + pattern);
    }
  }
}

void Executor::validateSchemaVersion(v_int64 currentVersion, v_int64 newVersion) {
  if (currentVersion < MIN_VERSION) {
    throw MigrationError("Current version is invalid: " + std::to_string(currentVersion));
  }
  
  if (newVersion <= currentVersion) {
    throw MigrationError("New version (" + std::to_string(newVersion) + 
                        ") must be greater than current version (" + 
                        std::to_string(currentVersion) + ")");
  }

  if (newVersion > MAX_VERSION) {
    throw MigrationError("New version exceeds maximum allowed value");
  }
}

void Executor::retryOnDeadlock(const std::function<void()>& operation) {
  const int MAX_RETRIES = 5;
  const int RETRY_DELAY_MS = 100;
  
  for(int attempt = 1; attempt <= MAX_RETRIES; ++attempt) {
    try {
      operation();
      return;
    } catch(const std::exception& e) {
      std::string error = e.what();
      if(error.find("Error 1213") != std::string::npos || // Deadlock found
         error.find("Error 1205") != std::string::npos) {  // Lock wait timeout
        if(attempt < MAX_RETRIES) {
          OATPP_LOGD("Executor", "Deadlock detected, attempt %d of %d. Retrying in %dms...", 
                     attempt, MAX_RETRIES, RETRY_DELAY_MS);
          std::this_thread::sleep_for(std::chrono::milliseconds(RETRY_DELAY_MS));
          continue;
        }
      }
      throw; // Re-throw if not a deadlock or max retries reached
    }
  }
  throw std::runtime_error("Max retry attempts reached while handling deadlock");
}

void Executor::acquireMigrationLock(const provider::ResourceHandle<orm::Connection>& connection,
                                  const oatpp::String& tableName,
                                  v_uint32 timeoutSeconds) {
  auto lockQuery = oatpp::String("SELECT GET_LOCK('migration_lock_") + tableName + "', " + 
                  std::to_string(timeoutSeconds) + ") as lock_status";
  auto result = executeRaw(lockQuery, connection);
  if (!result->isSuccess()) {
    throw std::runtime_error("Failed to acquire migration lock");
  }
  
  auto lockRow = result->fetch<oatpp::Object<LockResult>>();
  if (!lockRow || !lockRow->lock_status) {
    throw std::runtime_error("Failed to acquire migration lock - null value");
  }

  if (*lockRow->lock_status != 1) {
    throw std::runtime_error("Failed to acquire migration lock - timeout or error");
  }
}

void Executor::releaseMigrationLock(const provider::ResourceHandle<orm::Connection>& connection) {
  auto releaseQuery = "SELECT RELEASE_LOCK('migration_lock') as lock_status";
  auto result = executeRaw(releaseQuery, connection);
  if (!result->isSuccess()) {
    OATPP_LOGE("Executor", "Failed to release migration lock");
    return;
  }

  auto lockRow = result->fetch<oatpp::Object<LockResult>>();
  if (!lockRow || !lockRow->lock_status) {
    OATPP_LOGE("Executor", "Failed to release migration lock - null value");
    return;
  }

  if (*lockRow->lock_status != 1) {
    OATPP_LOGE("Executor", "Failed to release migration lock - error");
  }
}

void Executor::logMigrationError(const provider::ResourceHandle<orm::Connection>& connection,
                               const oatpp::String& tableName,
                               v_int64 version,
                               const std::string& error) {
  try {
    auto updateHistoryText = oatpp::String("UPDATE `") + tableName + 
                           "_history` SET status = 'failed', error = ?, " +
                           "completed_at = CURRENT_TIMESTAMP WHERE version = ? AND status = 'pending'";
    
    auto updateHistoryQuery = parseQueryTemplate("update_history_failed", updateHistoryText, {
      {"1", oatpp::String::Class::getType()},
      {"2", oatpp::Int64::Class::getType()}
    }, true);
    
    std::unordered_map<oatpp::String, oatpp::Void> params;
    params["1"] = oatpp::String(error.c_str());
    params["2"] = oatpp::Int64(version);
    
    execute(updateHistoryQuery, params, m_defaultTypeResolver, connection);
  } catch (const std::exception& e) {
    OATPP_LOGE("Executor", "Failed to log migration error: %s", e.what());
  }
}

void Executor::closeConnection(const provider::ResourceHandle<orm::Connection>& connection) {
  if (connection) {
    auto mariadbConnection = std::static_pointer_cast<Connection>(connection.object);
    if (mariadbConnection) {
      auto handle = mariadbConnection->getHandle();
      if (handle) {
        // Set shorter timeout for cleanup operations
        unsigned int timeout = 1;
        mysql_options(handle, MYSQL_OPT_READ_TIMEOUT, &timeout);
        mysql_options(handle, MYSQL_OPT_WRITE_TIMEOUT, &timeout);
        
        // Cancel any pending operations
        mysql_kill(handle, mysql_thread_id(handle));
        
        // Give a very brief moment for the kill to take effect
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        
        // Close the connection
        mysql_close(handle);
        
        // Set the connection handle to nullptr to prevent double-close
        mariadbConnection->setHandle(nullptr);
      }
    }
  }
}

void Executor::clearAllConnections() {
  if (m_connectionProvider) {
    auto provider = std::dynamic_pointer_cast<ConnectionProvider>(m_connectionProvider);
    if (provider) {
      provider->stop();
      provider->clear();
    }
  }
}

}}