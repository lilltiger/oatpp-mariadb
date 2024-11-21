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
                                                    const provider::ResourceHandle<orm::Connection>& connection)
{
  auto connectionHandle = connection;
  if (!connectionHandle) {
    connectionHandle = getConnection();
  }

  std::shared_ptr<const data::mapping::TypeResolver> tr = typeResolver;
  if(!tr) {
    tr = m_defaultTypeResolver;
  }

  auto mysqlConnection = std::static_pointer_cast<mariadb::Connection>(connectionHandle.object);

  MYSQL_STMT* stmt = mysql_stmt_init(mysqlConnection->getHandle());
  if (!stmt) {
    throw std::runtime_error("[oatpp::mariadb::Executor::execute()]: "
      "ErrorError. Can't create MYSQL_STMT. Error: " + std::string(mysql_error(mysqlConnection->getHandle())));
  }

  auto extra = std::static_pointer_cast<ql_template::Parser::TemplateExtra>(queryTemplate.getExtraData());
  if (mysql_stmt_prepare(stmt, extra->preparedTemplate->c_str(), extra->preparedTemplate->size())) {
    throw std::runtime_error("[oatpp::mariadb::Executor::execute()]: "
      "Error. Can't prepare MYSQL_STMT. preparedTemplate: " + extra->preparedTemplate +
      " Error: " + std::string(mysql_stmt_error(stmt)));
  }

  bindParams(stmt, queryTemplate, params, typeResolver);

  return std::make_shared<mariadb::QueryResult>(stmt, connectionHandle, m_resultMapper, tr);
}

std::shared_ptr<orm::QueryResult> Executor::begin(const provider::ResourceHandle<orm::Connection>& connection) {
  throw std::runtime_error("[oatpp::mariadb::Executor::begin()]: "
                           "Error. Not implemented.");
}

std::shared_ptr<orm::QueryResult> Executor::commit(const provider::ResourceHandle<orm::Connection>& connection) {
  throw std::runtime_error("[oatpp::mariadb::Executor::commit()]: "
                           "Error. Not implemented.");
}

std::shared_ptr<orm::QueryResult> Executor::rollback(const provider::ResourceHandle<orm::Connection>& connection) {
  throw std::runtime_error("[oatpp::mariadb::Executor::rollback()]: "
                           "Error. Not implemented.");
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

}}