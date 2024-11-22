#include "ResultMapper.hpp"

namespace oatpp { namespace mariadb { namespace mapping {

ResultMapper::ResultData::ResultData(MYSQL_STMT* pStmt, const std::shared_ptr<const data::mapping::TypeResolver>& pTypeResolver)
  : stmt(pStmt)
  , typeResolver(pTypeResolver)
{
  bindResultsForCache();
}

ResultMapper::ResultData::~ResultData() {
  // free bind results
  for (auto& bind : bindResults) {
    if (bind.buffer) {
      free(bind.buffer);
      bind.buffer = nullptr;
    }
    if (bind.is_null) {
      free(bind.is_null);
      bind.is_null = nullptr;
    }
  }

  if (metaResults) {
    mysql_free_result(metaResults);
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
  // if null, no result set
  if (metaResults) {
    colCount = mysql_num_fields(metaResults);
    MYSQL_FIELD* fields = mysql_fetch_fields(metaResults);

    for (v_int32 i = 0; i < colCount; i++) {
      oatpp::String colName = fields[i].name;
      colNames.push_back(colName);
      colIndices.insert({colName, i});

      // OATPP_LOGD("oatpp::mariadb::mapping::ResultMapper::ResultData::bindResultsForCache()", "Column %d: %s - %d", 
      //   i, colName->c_str(), fields[i].type);

      // bind result cache
      MYSQL_BIND bind;
      std::memset(&bind, 0, sizeof(bind));

      bind.buffer_type = fields[i].type;

      // indicate through is_null pointer if the value is null
      my_bool* is_null = static_cast<my_bool*>(malloc(sizeof(my_bool)));
      bind.is_null = is_null;

      if (fields[i].type == MYSQL_TYPE_TINY) {
        auto p_int8 = static_cast<int8_t*>(malloc(sizeof(int8_t)));
        bind.buffer = p_int8;
        bind.buffer_length = 0;
      }
      else if (fields[i].type == MYSQL_TYPE_SHORT) {
        auto p_int16 = static_cast<int16_t*>(malloc(sizeof(int16_t)));
        bind.buffer = p_int16;
        bind.buffer_length = 0;
      }
      else if (fields[i].type == MYSQL_TYPE_LONG) {
        auto p_int32 = static_cast<int32_t*>(malloc(sizeof(int32_t)));
        bind.buffer = p_int32;
        bind.buffer_length = 0;
      }
      else if (fields[i].type == MYSQL_TYPE_LONGLONG || fields[i].type == MYSQL_TYPE_TIMESTAMP2) {
        auto p_int64 = static_cast<int64_t*>(malloc(sizeof(int64_t)));
        bind.buffer_type = MYSQL_TYPE_LONGLONG;
        bind.buffer = p_int64;
        bind.buffer_length = 0;
      }
      else if (fields[i].type == MYSQL_TYPE_FLOAT) {
        auto p_float = static_cast<float*>(malloc(sizeof(float)));
        bind.buffer = p_float;
        bind.buffer_length = 0;
      }
      else if (fields[i].type == MYSQL_TYPE_DOUBLE) {
        auto p_double = static_cast<double*>(malloc(sizeof(double)));
        bind.buffer = p_double;
        bind.buffer_length = 0;
      }
      else if (fields[i].type == MYSQL_TYPE_STRING || fields[i].type == MYSQL_TYPE_VAR_STRING || 
               fields[i].type == MYSQL_TYPE_VARCHAR || fields[i].type == MYSQL_TYPE_DATETIME ||
               fields[i].type == MYSQL_TYPE_DATE) {
        auto p_string = static_cast<char*>(malloc(fields[i].length + 1));
        bind.buffer_type = MYSQL_TYPE_STRING;
        bind.buffer = p_string;
        bind.buffer_length = fields[i].length + 1;
      }
      else {
        throw std::runtime_error("[oatpp::mariadb::mapping::ResultMapper::ResultData::ResultData()]: Unknown field type: " 
          + std::string(fields[i].name) + " - " + std::to_string(fields[i].type));
      }

      bindResults.push_back(bind);
    }

    if (mysql_stmt_bind_result(stmt, bindResults.data())) {
      throw std::runtime_error("[oatpp::mariadb::mapping::ResultMapper::ResultData::ResultData()]: mysql_stmt_bind_result() failed");
    }
  }
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
