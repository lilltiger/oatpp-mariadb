#include "Executor.hpp"

#include "QueryResult.hpp"
#include "mapping/ResultMapper.hpp"

namespace oatpp { namespace mariadb {

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
      throw std::runtime_error("[oatpp::mariadb::Executor::bindParams()]: Error. "
        "Can't parse query parameter name. Parameter name: " + var.name);
    }

    // resolve parameter type
    auto it = params.find(queryParam.name);
    if (it != params.end()) {
      auto value = typeResolver->resolveObjectPropertyValue(it->second, queryParam.propertyPath, cache);
      if (value.getValueType()->classId.id == oatpp::Void::Class::CLASS_ID.id) {
        throw std::runtime_error("[oatpp::mariadb::Executor::bindParams()]: Error. "
          "Can't resolve parameter type because property dose not found or its type is unknown." 
          " Parameter name: " + queryParam.name + ", var.name: " + var.name);
      }

      // [serialize] bind parameter according to the resolved type
      m_serializer->serialize(stmt, i, value);
    }
  }

  if (mysql_stmt_bind_param(stmt, m_serializer->getBindParams().data())) {
    throw std::runtime_error("[oatpp::mariadb::Executor::bindParams()]: Error. "
      "Can't bind parameters. Error: " + std::string(mysql_stmt_error(stmt)));
  }
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
    throw std::runtime_error("[oatpp::mariadb::Executor::execute()]: Error. Unable to initialize statement.");
  }

  OATPP_LOGD("Executor", "Statement initialized. Address: %p", (void*)stmt);

  if (mysql_stmt_prepare(stmt, extra->preparedTemplate->c_str(), extra->preparedTemplate->size())) {
    std::string error = mysql_stmt_error(stmt);
    mysql_stmt_close(stmt);
    throw std::runtime_error("[oatpp::mariadb::Executor::execute()]: Error. Unable to prepare statement: " + error);
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
  return execute(queryTemplate, {}, nullptr, connection);
}

std::shared_ptr<orm::QueryResult> Executor::execute(const oatpp::String& query,
                                             const std::shared_ptr<const data::mapping::TypeResolver>& typeResolver,
                                             const provider::ResourceHandle<orm::Connection>& connection) {
  
  auto mysqlConnection = std::static_pointer_cast<mariadb::Connection>(connection.object);
  if (!mysqlConnection) {
    throw std::runtime_error("[oatpp::mariadb::Executor::execute]: Invalid connection");
  }

  MYSQL* mysql = mysqlConnection->getHandle();
  if (!mysql) {
    throw std::runtime_error("[oatpp::mariadb::Executor::execute]: MySQL connection handle is null");
  }

  MYSQL_STMT* stmt = mysql_stmt_init(mysql);
  if (!stmt) {
    throw std::runtime_error("[oatpp::mariadb::Executor::execute]: Failed to initialize statement");
  }

  if (mysql_stmt_prepare(stmt, query->c_str(), query->size())) {
    auto error = std::string(mysql_stmt_error(stmt));
    mysql_stmt_close(stmt);
    throw std::runtime_error("[oatpp::mariadb::Executor::execute]: Statement prepare error: " + error);
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
    throw std::runtime_error("[oatpp::mariadb::Executor::executeRaw()]: "
      "Error. Can't create MYSQL_STMT. Error: " + std::string(mysql_error(mysqlConnection->getHandle())));
  }

  if (mysql_stmt_prepare(stmt, query->c_str(), query->size())) {
    throw std::runtime_error("[oatpp::mariadb::Executor::executeRaw()]: "
      "Error. Can't prepare MYSQL_STMT. Query: " + query +
      " Error: " + std::string(mysql_stmt_error(stmt)));
  }

  if (mysql_stmt_execute(stmt)) {
    throw std::runtime_error("[oatpp::mariadb::Executor::executeRaw()]: "
      "Error. Can't execute MYSQL_STMT. Query: " + query +
      " Error: " + std::string(mysql_stmt_error(stmt)));
  }

  return std::make_shared<mariadb::QueryResult>(stmt, connectionHandle, m_resultMapper, nullptr);
}

std::shared_ptr<orm::QueryResult> Executor::begin(const provider::ResourceHandle<orm::Connection>& connection) {
  return execute("START TRANSACTION", nullptr, connection);
}

std::shared_ptr<orm::QueryResult> Executor::commit(const provider::ResourceHandle<orm::Connection>& connection) {
  return execute("COMMIT", nullptr, connection);
}

std::shared_ptr<orm::QueryResult> Executor::rollback(const provider::ResourceHandle<orm::Connection>& connection) {
  return execute("ROLLBACK", nullptr, connection);
}

void Executor::rollbackToSavepoint(const provider::ResourceHandle<orm::Connection>& connection, const String& savepointName) {
  auto query = String("ROLLBACK TO SAVEPOINT ") + savepointName;
  execute(query, nullptr, connection);
}

void Executor::setSavepoint(const provider::ResourceHandle<orm::Connection>& connection, const String& savepointName) {
  auto query = String("SAVEPOINT ") + savepointName;
  execute(query, nullptr, connection);
}

void Executor::releaseSavepoint(const provider::ResourceHandle<orm::Connection>& connection, const String& savepointName) {
  auto query = String("RELEASE SAVEPOINT ") + savepointName;
  execute(query, nullptr, connection);
}

v_int64 Executor::getSchemaVersion(const oatpp::String& suffix,
                                   const provider::ResourceHandle<orm::Connection>& connection)
{
  throw std::runtime_error("[oatpp::mariadb::Executor::getSchemaVersion()]: "
                           "Error. Not implemented.");
}

void Executor::migrateSchema(const oatpp::String& script,
                             v_int64 newVersion,
                             const oatpp::String& suffix,
                             const provider::ResourceHandle<orm::Connection>& connection)
{
  throw std::runtime_error("[oatpp::mariadb::Executor::migrateSchema()]: "
                           "Error. Not implemented.");
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