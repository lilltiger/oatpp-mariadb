#include "ResultMapper.hpp"

namespace oatpp { namespace mariadb { namespace mapping {

ResultMapper::ResultData::ResultData(MYSQL_STMT* pStmt, const std::shared_ptr<const data::mapping::TypeResolver>& pTypeResolver)
  : stmt(pStmt)
  , typeResolver(pTypeResolver)
{
  bindResultsForCache();
}

ResultMapper::ResultData::~ResultData() {
  // Free bind results
  for (auto& bind : bindResults) {
    if (bind.buffer) {
      free(bind.buffer);
      bind.buffer = nullptr;
    }
    if (bind.is_null && bind.is_null != &bind.is_null_value) {
      free(bind.is_null);
      bind.is_null = nullptr;
    }
    if (bind.length && bind.length != &bind.length_value) {
      free(bind.length);
      bind.length = nullptr;
    }
    if (bind.error && bind.error != &bind.error_value) {
      free(bind.error);
      bind.error = nullptr;
    }
  }

  // Clear the vectors to ensure no dangling pointers
  bindResults.clear();
  colNames.clear();
  colIndices.clear();

  if (metaResults) {
    mysql_free_result(metaResults);
    metaResults = nullptr;
  }
}

void ResultMapper::ResultData::init() {
  next();
  rowIndex = 0;
}

void ResultMapper::ResultData::next() {
  auto res = mysql_stmt_fetch(stmt);

  switch(res) {
    // fetch row failed
    case 1: {
      hasMore = false;
      isSuccess = false;
    }
    // no more rows
    case MYSQL_NO_DATA: {
      hasMore = false;
      isSuccess = true;
      break;
    };
    // data truncated
    case MYSQL_DATA_TRUNCATED: {
      // TODO: handle data truncation
    }
    // fetch row success
    default: {
      hasMore = true;
      isSuccess = true;
    }

  }

}

void ResultMapper::ResultData::bindResultsForCache() {
  metaResults = mysql_stmt_result_metadata(stmt);
  if (!metaResults) {
    isSuccess = true;
    hasMore = false;
    return;
  }

  colCount = mysql_num_fields(metaResults);
  if (colCount <= 0) {
    isSuccess = true;
    hasMore = false;
    return;
  }

  // Resize bindResults vector to match column count
  bindResults.resize(colCount);
  
  // Initialize all bind structures to zero
  for (auto& bind : bindResults) {
    std::memset(&bind, 0, sizeof(MYSQL_BIND));
    bind.is_null_value = 0;
    bind.error_value = 0;
    bind.length_value = 0;
  }

  // Get field information and set up bindings
  MYSQL_FIELD* fields = mysql_fetch_fields(metaResults);
  for (unsigned int i = 0; i < colCount; i++) {
    auto& bind = bindResults[i];
    
    // Use the stack-allocated values by default
    bind.is_null = &bind.is_null_value;
    bind.error = &bind.error_value;
    bind.length = &bind.length_value;

    // Set up the binding based on field type
    switch (fields[i].type) {
      case MYSQL_TYPE_TINY:
        bind.buffer_type = MYSQL_TYPE_TINY;
        bind.buffer = malloc(sizeof(char));
        bind.buffer_length = sizeof(char);
        break;
        
      case MYSQL_TYPE_SHORT:
        bind.buffer_type = MYSQL_TYPE_SHORT;
        bind.buffer = malloc(sizeof(short));
        bind.buffer_length = sizeof(short);
        break;
        
      case MYSQL_TYPE_LONG:
      case MYSQL_TYPE_INT24:
        bind.buffer_type = MYSQL_TYPE_LONG;
        bind.buffer = malloc(sizeof(int));
        bind.buffer_length = sizeof(int);
        break;
        
      case MYSQL_TYPE_LONGLONG:
        bind.buffer_type = MYSQL_TYPE_LONGLONG;
        bind.buffer = malloc(sizeof(long long));
        bind.buffer_length = sizeof(long long);
        break;
        
      case MYSQL_TYPE_FLOAT:
        bind.buffer_type = MYSQL_TYPE_FLOAT;
        bind.buffer = malloc(sizeof(float));
        bind.buffer_length = sizeof(float);
        break;
        
      case MYSQL_TYPE_DOUBLE:
        bind.buffer_type = MYSQL_TYPE_DOUBLE;
        bind.buffer = malloc(sizeof(double));
        bind.buffer_length = sizeof(double);
        break;
        
      case MYSQL_TYPE_STRING:
      case MYSQL_TYPE_VAR_STRING:
      case MYSQL_TYPE_BLOB:
      default:
        bind.buffer_type = MYSQL_TYPE_STRING;
        // Add extra byte for null terminator and ensure minimum size
        bind.buffer_length = std::max(fields[i].length + 1, (unsigned long)256);
        bind.buffer = malloc(bind.buffer_length);
        break;
    }
    
    if (!bind.buffer) {
      throw std::runtime_error("Failed to allocate buffer memory");
    }

    // Store column name
    oatpp::String colName = fields[i].name;
    colNames.push_back(colName);
    colIndices[data::share::StringKeyLabel(colName)] = i;
  }

  // Bind the result buffers
  if (mysql_stmt_bind_result(stmt, bindResults.data())) {
    throw std::runtime_error(mysql_stmt_error(stmt));
  }

  isSuccess = true;
  hasMore = true;
}

ResultMapper::ResultMapper() {

  {
    m_readOneRowMethods.resize(data::mapping::type::ClassId::getClassCount(), nullptr);

    // object
    setReadOneRowMethod(data::mapping::type::__class::AbstractObject::CLASS_ID, &ResultMapper::readOneRowAsObject);

    // collection
    setReadOneRowMethod(data::mapping::type::__class::AbstractVector::CLASS_ID, &ResultMapper::readOneRowAsCollection);
    setReadOneRowMethod(data::mapping::type::__class::AbstractList::CLASS_ID, &ResultMapper::readOneRowAsCollection);
    setReadOneRowMethod(data::mapping::type::__class::AbstractUnorderedSet::CLASS_ID,
                        &ResultMapper::readOneRowAsCollection);

    // map
    setReadOneRowMethod(data::mapping::type::__class::AbstractPairList::CLASS_ID, &ResultMapper::readOneRowAsMap);
    setReadOneRowMethod(data::mapping::type::__class::AbstractUnorderedMap::CLASS_ID, &ResultMapper::readOneRowAsMap);
  }

  {
    m_readRowsMethods.resize(data::mapping::type::ClassId::getClassCount(), nullptr);

    // collection
    setReadRowsMethod(data::mapping::type::__class::AbstractVector::CLASS_ID, &ResultMapper::readRowsAsCollection);
    setReadRowsMethod(data::mapping::type::__class::AbstractList::CLASS_ID, &ResultMapper::readRowsAsCollection);
    setReadRowsMethod(data::mapping::type::__class::AbstractUnorderedSet::CLASS_ID, &ResultMapper::readRowsAsCollection);

    // object
    setReadRowsMethod(data::mapping::type::__class::AbstractObject::CLASS_ID, &ResultMapper::readRowsAsObject);
  }

}

void ResultMapper::setReadOneRowMethod(const data::mapping::type::ClassId& classId, ReadOneRowMethod method) {
  const v_uint32 id = classId.id;
  if(id >= m_readOneRowMethods.size()) {
    m_readOneRowMethods.resize(id + 1, nullptr);
  }
  m_readOneRowMethods[id] = method;
}

void ResultMapper::setReadRowsMethod(const data::mapping::type::ClassId& classId, ReadRowsMethod method) {
  const v_uint32 id = classId.id;
  if(id >= m_readRowsMethods.size()) {
    m_readRowsMethods.resize(id + 1, nullptr);
  }
  m_readRowsMethods[id] = method;
}

oatpp::Void ResultMapper::readOneRowAsCollection(ResultMapper* _this, ResultData* dbData, const Type* type) {

  auto dispatcher = static_cast<const data::mapping::type::__class::Collection::PolymorphicDispatcher*>(type->polymorphicDispatcher);
  auto collection = dispatcher->createObject();

  const Type* itemType = dispatcher->getItemType();

  for(v_int32 i = 0; i < dbData->colCount; i ++) {
    mapping::Deserializer::InData inData(&dbData->bindResults[i], dbData->typeResolver);
    // get one column data and deserialize it according to the itemType
    dispatcher->addItem(collection, _this->m_deserializer.deserialize(inData, itemType));
  }

  return collection;

}

oatpp::Void ResultMapper::readOneRowAsMap(ResultMapper* _this, ResultData* dbData, const Type* type) {

  auto dispatcher = static_cast<const data::mapping::type::__class::Map::PolymorphicDispatcher*>(type->polymorphicDispatcher);
  auto map = dispatcher->createObject();

  const Type* keyType = dispatcher->getKeyType();
  if(keyType->classId.id != oatpp::data::mapping::type::__class::String::CLASS_ID.id){
    throw std::runtime_error("[oatpp::mariadb::mapping::ResultMapper::readOneRowAsMap()]: Invalid map key. Key should be String");
  }

  const Type* valueType = dispatcher->getValueType();
  for(v_int32 i = 0; i < dbData->colCount; i ++) {
    mapping::Deserializer::InData inData(&dbData->bindResults[i], dbData->typeResolver);
    dispatcher->addItem(map, dbData->colNames[i], _this->m_deserializer.deserialize(inData, valueType));
  }

  return map;

}

oatpp::Void ResultMapper::readOneRowAsObject(ResultMapper* _this, ResultData* dbData, const Type* type) {

  auto dispatcher = static_cast<const data::mapping::type::__class::AbstractObject::PolymorphicDispatcher*>(type->polymorphicDispatcher);
  auto object = dispatcher->createObject();
  const auto& fieldsMap = dispatcher->getProperties()->getMap();

  std::vector<std::pair<oatpp::BaseObject::Property*, v_int32>> polymorphs;

  for(v_int32 i = 0; i < dbData->colCount; i ++) {

    auto it = fieldsMap.find(*dbData->colNames[i]);

    if(it != fieldsMap.end()) {
      auto field = it->second;
      if(field->info.typeSelector && field->type == oatpp::Any::Class::getType()) {
        // OATPP_LOGD("[oatpp::mariadb::mapping::ResultMapper::readOneRowAsObject]", "polymorphic field=%s, index=%d", field->name, i);
        polymorphs.push_back({field, i});
      } else {
        // OATPP_LOGD("[oatpp::mariadb::mapping::ResultMapper::readOneRowAsObject]", "field=%s, index=%d", field->name, i);
        mapping::Deserializer::InData inData(&dbData->bindResults[i], dbData->typeResolver);
        field->set(static_cast<oatpp::BaseObject *>(object.get()),
                   _this->m_deserializer.deserialize(inData, field->type));
      }
    } else {
      OATPP_LOGE("[oatpp::mariadb::mapping::ResultMapper::readOneRowAsObject]",
                 "Error. The object of type '%s' has no field to map column '%s'.",
                 type->nameQualifier, dbData->colNames[i]->c_str());
      throw std::runtime_error("[oatpp::mariadb::mapping::ResultMapper::readOneRowAsObject]: Error. "
                               "The object of type " + std::string(type->nameQualifier) +
                               " has no field to map column " + *dbData->colNames[i]  + ".");
    }

  }

  for(auto& p : polymorphs) {
    v_int32 index = p.second;
    mapping::Deserializer::InData inData(&dbData->bindResults[index], dbData->typeResolver);
    auto selectedType = p.first->info.typeSelector->selectType(static_cast<oatpp::BaseObject *>(object.get()));
    auto value = _this->m_deserializer.deserialize(inData, selectedType);
    oatpp::Any any(value);
    p.first->set(static_cast<oatpp::BaseObject *>(object.get()), oatpp::Void(any.getPtr(), p.first->type));
  }

  return object;

}

oatpp::Void ResultMapper::readRowsAsCollection(ResultMapper* _this, ResultData* dbData, const Type* type, v_int64 count) {

  auto dispatcher = static_cast<const data::mapping::type::__class::Collection::PolymorphicDispatcher*>(type->polymorphicDispatcher);
  auto collection = dispatcher->createObject();   // empty collection

  if(count != 0) {

    const Type *itemType = *type->params.begin();

    v_int64 counter = 0;
    while (dbData->hasMore) {
      // get one row data and deserialize it according to the itemType
      dispatcher->addItem(collection, _this->readOneRow(dbData, itemType));
      ++dbData->rowIndex;
      dbData->next();
      if (count > 0) {
        ++counter;
        if (counter == count) {
          break;
        }
      }
    }

  }

  return collection;

}

oatpp::Void ResultMapper::readRowsAsObject(ResultMapper* _this, ResultData* dbData, const Type* type, v_int64 count) {
  if (count > 1) {
    throw std::runtime_error("[oatpp::mariadb::mapping::ResultMapper::readRowsAsObject()]: "
                           "Error. Cannot read multiple rows into a single object.");
  }
  
  if (!dbData->hasMore) {
    return nullptr;
  }
  
  auto result = _this->readOneRow(dbData, type);
  ++dbData->rowIndex;
  dbData->next();
  return result;
}

oatpp::Void ResultMapper::readOneRow(ResultData* dbData, const Type* type) {

  auto id = type->classId.id;
  auto& method = m_readOneRowMethods[id];

  // OATPP_LOGD("[oatpp::mariadb::mapping::ResultMapper::readOneRow]", "type=%s, method=%p", type->nameQualifier, method);

  if(method) {
    return (*method)(this, dbData, type);
  }

  // if no method found - try to find interpretation
  auto* interpretation = type->findInterpretation(dbData->typeResolver->getEnabledInterpretations());
  if(interpretation) {
    return interpretation->fromInterpretation(readOneRow(dbData, interpretation->getInterpretationType()));
  }

  throw std::runtime_error("[oatpp::mariadb::mapping::ResultMapper::readOneRow()]: "
                           "Error. Invalid result container type. "
                           "Allowed types are "
                           "oatpp::Vector, "
                           "oatpp::List, "
                           "oatpp::UnorderedSet, "
                           "oatpp::Fields, "
                           "oatpp::UnorderedFields, "
                           "oatpp::Object");

}

oatpp::Void ResultMapper::readRows(ResultData* dbData, const Type* type, v_int64 count) {

  auto id = type->classId.id;
  auto& method = m_readRowsMethods[id];

  // OATPP_LOGD("[oatpp::mariadb::mapping::ResultMapper::readRows]", "type=%s, method=%p, id=%d", type->classId.name, method, id);

  if(method) {
    return (*method)(this, dbData, type, count);
  }

  throw std::runtime_error("[oatpp::mariadb::mapping::ResultMapper::readRows()]: "
                           "Error. Invalid result container type. "
                           "Allowed types are oatpp::Vector, oatpp::List, oatpp::UnorderedSet");

}

}}}
