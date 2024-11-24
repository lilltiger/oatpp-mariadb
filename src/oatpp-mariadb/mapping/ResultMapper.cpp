#include "ResultMapper.hpp"

namespace oatpp { namespace mariadb { namespace mapping {

ResultMapper::ResultData::ResultData(MYSQL_STMT* pStmt, const std::shared_ptr<const data::mapping::TypeResolver>& pTypeResolver)
  : stmt(pStmt)
  , typeResolver(pTypeResolver)
  , colCount(0)
  , rowIndex(0)
  , hasMore(false)
  , isSuccess(false)
  , bindResults()
  , bindIsNull()
  , bindLengths()
  , bindBuffers()
  , metaResults(nullptr)
{
  if (!stmt) {
    OATPP_LOGE("ResultMapper", "Error: stmt is null in constructor");
    return;
  }

  metaResults = mysql_stmt_result_metadata(stmt);
  if (metaResults) {
    colCount = mysql_num_fields(metaResults);
    bindResultsForCache();
  }
}

ResultMapper::ResultData::~ResultData() {
  // Clear the vectors to ensure no dangling pointers
  bindResults.clear();
  colNames.clear();
  colIndices.clear();
  bindIsNull.clear();
  bindLengths.clear();
  bindBuffers.clear();

  if (metaResults) {
    mysql_free_result(metaResults);
    metaResults = nullptr;
  }
}

void ResultMapper::ResultData::init() {
  if (!stmt) {
    OATPP_LOGE("ResultMapper", "Error: stmt is null in init()");
    isSuccess = false;
    hasMore = false;
    return;
  }

  // For non-SELECT queries, metaResults will be null and that's okay
  if (!metaResults) {
    OATPP_LOGD("ResultMapper", "No metadata in init() (normal for non-SELECT queries)");
    
    // For INSERT...RETURNING queries, we still need to fetch the result
    int result = mysql_stmt_store_result(stmt);
    if (result != 0) {
      OATPP_LOGE("ResultMapper", "Failed to store result: %s", mysql_stmt_error(stmt));
      isSuccess = false;
      hasMore = false;
      return;
    }
    
    // Check if we have any rows
    my_ulonglong rows = mysql_stmt_num_rows(stmt);
    hasMore = (rows > 0);
    OATPP_LOGD("ResultMapper", "Found %llu rows in result", rows);

    // For RETURNING clauses, we need to initialize column names and bindings
    MYSQL_RES* prepare_meta_result = mysql_stmt_result_metadata(stmt);
    if (prepare_meta_result) {
      OATPP_LOGD("ResultMapper", "Found metadata for RETURNING clause");
      colCount = mysql_num_fields(prepare_meta_result);
      MYSQL_FIELD* fields = mysql_fetch_fields(prepare_meta_result);
      
      // Initialize column names and indices
      colNames.clear();
      colIndices.clear();

      // Initialize bindings for result set
      bindResults.resize(colCount);
      bindIsNull.resize(colCount);
      bindLengths.resize(colCount);
      bindBuffers.resize(colCount);

      for (unsigned int i = 0; i < colCount; i++) {
        oatpp::String colName = fields[i].name;
        colNames.push_back(colName);
        colIndices[data::share::StringKeyLabel(colName)] = i;
        OATPP_LOGD("ResultMapper", "Initialized column %d: %s (type: %d)", i, colName->c_str(), fields[i].type);

        // Initialize binding
        MYSQL_BIND& bind = bindResults[i];
        std::memset(&bind, 0, sizeof(bind));
        bind.is_null = &bindIsNull[i];
        bind.length = &bindLengths[i];

        // Map field type to appropriate buffer type and size
        size_t bufferSize;
        switch (fields[i].type) {
          case MYSQL_TYPE_TINY:
            bind.buffer_type = MYSQL_TYPE_TINY;
            bufferSize = sizeof(int8_t);
            break;
          case MYSQL_TYPE_SHORT:
            bind.buffer_type = MYSQL_TYPE_SHORT;
            bufferSize = sizeof(int16_t);
            break;
          case MYSQL_TYPE_LONG:
          case MYSQL_TYPE_INT24:
            bind.buffer_type = MYSQL_TYPE_LONG;
            bufferSize = sizeof(int32_t);
            break;
          case MYSQL_TYPE_LONGLONG:
            bind.buffer_type = MYSQL_TYPE_LONGLONG;
            bufferSize = sizeof(int64_t);
            break;
          case MYSQL_TYPE_FLOAT:
            bind.buffer_type = MYSQL_TYPE_FLOAT;
            bufferSize = sizeof(float);
            break;
          case MYSQL_TYPE_DOUBLE:
          case MYSQL_TYPE_DECIMAL:
          case MYSQL_TYPE_NEWDECIMAL:
            bind.buffer_type = MYSQL_TYPE_DOUBLE;
            bufferSize = sizeof(double);
            break;
          case MYSQL_TYPE_STRING:
          case MYSQL_TYPE_VAR_STRING:
          case MYSQL_TYPE_VARCHAR:
            bind.buffer_type = MYSQL_TYPE_STRING;
            bufferSize = fields[i].length + 1;  // Add 1 for null terminator
            break;
          case MYSQL_TYPE_DATE:
            bind.buffer_type = MYSQL_TYPE_STRING;
            bufferSize = 11;  // YYYY-MM-DD + null terminator
            break;
          default:
            bind.buffer_type = MYSQL_TYPE_STRING;
            bufferSize = fields[i].length + 1;
            break;
        }

        bindBuffers[i].resize(bufferSize);
        bind.buffer = bindBuffers[i].data();
        bind.buffer_length = bufferSize;
        
        // Zero out numeric buffers
        if (bind.buffer_type == MYSQL_TYPE_TINY ||
            bind.buffer_type == MYSQL_TYPE_SHORT ||
            bind.buffer_type == MYSQL_TYPE_LONG ||
            bind.buffer_type == MYSQL_TYPE_LONGLONG ||
            bind.buffer_type == MYSQL_TYPE_FLOAT ||
            bind.buffer_type == MYSQL_TYPE_DOUBLE) {
          std::memset(bind.buffer, 0, bufferSize);
        }
      }

      // Bind the result buffers
      if (mysql_stmt_bind_result(stmt, bindResults.data()) != 0) {
        OATPP_LOGE("ResultMapper", "Failed to bind result: %s", mysql_stmt_error(stmt));
        isSuccess = false;
        hasMore = false;
        mysql_free_result(prepare_meta_result);
        return;
      }

      // Fetch first row after binding
      int fetchResult = mysql_stmt_fetch(stmt);
      if (fetchResult == 0) {
        hasMore = true;
        isSuccess = true;
      } else if (fetchResult == MYSQL_NO_DATA) {
        hasMore = false;
        isSuccess = true;
        OATPP_LOGD("ResultMapper", "No rows to fetch after binding");
      } else {
        OATPP_LOGE("ResultMapper", "Error fetching first row: %s", mysql_stmt_error(stmt));
        isSuccess = false;
        hasMore = false;
        mysql_free_result(prepare_meta_result);
        return;
      }

      mysql_free_result(prepare_meta_result);
    } else {
      OATPP_LOGD("ResultMapper", "No metadata for RETURNING clause");
      isSuccess = true;  // Still consider it success even without RETURNING metadata
    }

    return;
  }

  MYSQL_FIELD* fields = mysql_fetch_fields(metaResults);
  if (!fields) {
    OATPP_LOGE("ResultMapper", "Error: Failed to fetch fields in init()");
    isSuccess = false;
    hasMore = false;
    return;
  }

  // Initialize column names and indices
  colNames.clear();
  colIndices.clear();

  for (unsigned int i = 0; i < colCount; i++) {
    oatpp::String colName = fields[i].name;
    colNames.push_back(colName);
    colIndices[data::share::StringKeyLabel(colName)] = i;
    OATPP_LOGD("ResultMapper", "Initialized column %d: %s", i, colName->c_str());
  }

  // Store the result set
  int result = mysql_stmt_store_result(stmt);
  if (result != 0) {
    OATPP_LOGE("ResultMapper", "Failed to store result: %s", mysql_stmt_error(stmt));
    isSuccess = false;
    hasMore = false;
    return;
  }

  // Check if we have any rows
  my_ulonglong rows = mysql_stmt_num_rows(stmt);
  hasMore = (rows > 0);
  OATPP_LOGD("ResultMapper", "Found %llu rows in result", rows);

  if (hasMore) {
    // Fetch first row
    result = mysql_stmt_fetch(stmt);
    if (result == 0) {
      hasMore = true;
      isSuccess = true;  // Set success flag when first row is fetched successfully
    } else if (result == MYSQL_NO_DATA) {
      hasMore = false;
      isSuccess = true;  // Still consider it success even with no data
    } else {
      OATPP_LOGE("ResultMapper", "Error fetching first row: %s", mysql_stmt_error(stmt));
      isSuccess = false;
      hasMore = false;
    }
  } else {
    isSuccess = true;  // Consider it success even with no rows
  }
}

void ResultMapper::ResultData::next() {
  if (!stmt || !hasMore) {
    OATPP_LOGD("ResultMapper", "No more rows to fetch");
    hasMore = false;
    return;
  }

  auto res = mysql_stmt_fetch(stmt);
  if (res == 0) {
    rowIndex++;
    hasMore = true;
  } else if (res == MYSQL_NO_DATA) {
    OATPP_LOGD("ResultMapper", "No more data");
    hasMore = false;
  } else {
    OATPP_LOGE("ResultMapper", "Error fetching next row: %s", mysql_stmt_error(stmt));
    hasMore = false;
    isSuccess = false;
  }
}

void ResultMapper::ResultData::bindResultsForCache() {
  if (!stmt) {
    throw std::runtime_error("Statement is null");
  }

  MYSQL_RES* metaData = mysql_stmt_result_metadata(stmt);
  if (!metaData) {
    OATPP_LOGD("ResultMapper", "No metadata in bindResultsForCache() (normal for non-SELECT queries)");
    return;
  }

  unsigned int numFields = mysql_num_fields(metaData);
  MYSQL_FIELD* fields = mysql_fetch_fields(metaData);

  bindResults.resize(numFields);
  bindIsNull.resize(numFields);
  bindLengths.resize(numFields);
  bindBuffers.resize(numFields);

  for(unsigned int i = 0; i < numFields; i++) {
    auto fieldInfo = std::make_shared<ResultMapper::FieldInfo>(
      fields[i].name,
      fields[i].type,
      fields[i].flags & UNSIGNED_FLAG,
      fields[i].length
    );
    
    OATPP_LOGD("ResultMapper", "Binding field '%s' of type %d", fieldInfo->name.c_str(), fieldInfo->type);
    
    std::memset(&bindResults[i], 0, sizeof(MYSQL_BIND));
    bindResults[i].is_null = &bindIsNull[i];
    bindResults[i].length = &bindLengths[i];
    
    switch(fieldInfo->type) {
      case MYSQL_TYPE_TINY:
        bindBuffers[i].resize(sizeof(int8_t));
        bindResults[i].buffer_type = MYSQL_TYPE_TINY;
        bindResults[i].buffer = bindBuffers[i].data();
        bindResults[i].is_unsigned = fieldInfo->isUnsigned;
        bindResults[i].buffer_length = sizeof(int8_t);
        break;
        
      case MYSQL_TYPE_SHORT:
        bindBuffers[i].resize(sizeof(int16_t));
        bindResults[i].buffer_type = MYSQL_TYPE_SHORT;
        bindResults[i].buffer = bindBuffers[i].data();
        bindResults[i].is_unsigned = fieldInfo->isUnsigned;
        bindResults[i].buffer_length = sizeof(int16_t);
        break;
        
      case MYSQL_TYPE_INT24:
      case MYSQL_TYPE_LONG:
        bindBuffers[i].resize(sizeof(int32_t));
        bindResults[i].buffer_type = MYSQL_TYPE_LONG;
        bindResults[i].buffer = bindBuffers[i].data();
        bindResults[i].is_unsigned = fieldInfo->isUnsigned;
        bindResults[i].buffer_length = sizeof(int32_t);
        break;
        
      case MYSQL_TYPE_LONGLONG:
        bindBuffers[i].resize(sizeof(int64_t));
        bindResults[i].buffer_type = MYSQL_TYPE_LONGLONG;
        bindResults[i].buffer = bindBuffers[i].data();
        bindResults[i].is_unsigned = fieldInfo->isUnsigned;
        bindResults[i].buffer_length = sizeof(int64_t);
        break;
        
      case MYSQL_TYPE_FLOAT:
        bindBuffers[i].resize(sizeof(float));
        bindResults[i].buffer_type = MYSQL_TYPE_FLOAT;
        bindResults[i].buffer = bindBuffers[i].data();
        bindResults[i].buffer_length = sizeof(float);
        break;
        
      case MYSQL_TYPE_DOUBLE:
        bindBuffers[i].resize(sizeof(double));
        bindResults[i].buffer_type = MYSQL_TYPE_DOUBLE;
        bindResults[i].buffer = bindBuffers[i].data();
        bindResults[i].buffer_length = sizeof(double);
        break;
        
      case MYSQL_TYPE_STRING:
      case MYSQL_TYPE_VAR_STRING:
      case MYSQL_TYPE_VARCHAR:
        bindBuffers[i].resize(fieldInfo->columnLength);
        bindResults[i].buffer_type = MYSQL_TYPE_STRING;
        bindResults[i].buffer = bindBuffers[i].data();
        bindResults[i].buffer_length = fieldInfo->columnLength;
        break;
      case MYSQL_TYPE_DATE:
        bindBuffers[i].resize(11);  // YYYY-MM-DD + null terminator
        bindResults[i].buffer_type = MYSQL_TYPE_STRING;
        bindResults[i].buffer = bindBuffers[i].data();
        bindResults[i].buffer_length = 11;
        break;
      default:
        throw std::runtime_error("Buffer type is not supported");
    }
  }

  if(mysql_stmt_bind_result(stmt, bindResults.data())) {
    throw std::runtime_error(mysql_stmt_error(stmt));
  }

  mysql_free_result(metaData);
}

void ResultMapper::initBind(MYSQL_BIND& bind, const std::shared_ptr<FieldInfo>& fieldInfo) {
  OATPP_LOGD("ResultMapper", "Initializing bind for field '%s' of type %d", fieldInfo->name.c_str(), fieldInfo->type);
  std::memset(&bind, 0, sizeof(MYSQL_BIND));
  
  bind.is_null = (my_bool*)malloc(sizeof(my_bool));
  if(!bind.is_null) {
    throw std::runtime_error("Failed to allocate memory for is_null indicator");
  }
  *bind.is_null = 0;
  
  switch(fieldInfo->type) {
    
    case MYSQL_TYPE_TINY: {
      bind.buffer_type = MYSQL_TYPE_TINY;
      bind.buffer = malloc(sizeof(int8_t));
      if(!bind.buffer) {
        free(bind.is_null);
        throw std::runtime_error("Failed to allocate memory for TINY buffer");
      }
      bind.is_unsigned = fieldInfo->isUnsigned;
      bind.buffer_length = sizeof(int8_t);
      break;
    }
      
    case MYSQL_TYPE_SHORT: {
      bind.buffer_type = MYSQL_TYPE_SHORT;
      bind.buffer = malloc(sizeof(int16_t));
      if(!bind.buffer) {
        free(bind.is_null);
        throw std::runtime_error("Failed to allocate memory for SHORT buffer");
      }
      bind.is_unsigned = fieldInfo->isUnsigned;
      bind.buffer_length = sizeof(int16_t);
      break;
    }
      
    case MYSQL_TYPE_INT24:
    case MYSQL_TYPE_LONG: {
      bind.buffer_type = MYSQL_TYPE_LONG;
      bind.buffer = malloc(sizeof(int32_t));
      if(!bind.buffer) {
        free(bind.is_null);
        throw std::runtime_error("Failed to allocate memory for LONG buffer");
      }
      bind.is_unsigned = fieldInfo->isUnsigned;
      bind.buffer_length = sizeof(int32_t);
      break;
    }
      
    case MYSQL_TYPE_LONGLONG: {
      bind.buffer_type = MYSQL_TYPE_LONGLONG;
      bind.buffer = malloc(sizeof(int64_t));
      if(!bind.buffer) {
        free(bind.is_null);
        throw std::runtime_error("Failed to allocate memory for LONGLONG buffer");
      }
      bind.is_unsigned = fieldInfo->isUnsigned;
      bind.buffer_length = sizeof(int64_t);
      break;
    }
      
    case MYSQL_TYPE_FLOAT: {
      bind.buffer_type = MYSQL_TYPE_FLOAT;
      bind.buffer = malloc(sizeof(float));
      if(!bind.buffer) {
        free(bind.is_null);
        throw std::runtime_error("Failed to allocate memory for FLOAT buffer");
      }
      bind.buffer_length = sizeof(float);
      break;
    }
      
    case MYSQL_TYPE_DOUBLE: {
      bind.buffer_type = MYSQL_TYPE_DOUBLE;
      bind.buffer = malloc(sizeof(double));
      if(!bind.buffer) {
        free(bind.is_null);
        throw std::runtime_error("Failed to allocate memory for DOUBLE buffer");
      }
      bind.buffer_length = sizeof(double);
      break;
    }
      
    case MYSQL_TYPE_STRING:
    case MYSQL_TYPE_VAR_STRING:
    case MYSQL_TYPE_BLOB:
    case MYSQL_TYPE_TINY_BLOB:
    case MYSQL_TYPE_MEDIUM_BLOB:
    case MYSQL_TYPE_LONG_BLOB: {
      bind.buffer_type = MYSQL_TYPE_STRING;
      bind.buffer = malloc(fieldInfo->columnLength + 1);
      if(!bind.buffer) {
        free(bind.is_null);
        throw std::runtime_error("Failed to allocate memory for STRING/BLOB buffer");
      }
      bind.buffer_length = fieldInfo->columnLength;
      bind.length = (unsigned long*)malloc(sizeof(unsigned long));
      if(!bind.length) {
        free(bind.buffer);
        free(bind.is_null);
        throw std::runtime_error("Failed to allocate memory for STRING/BLOB length");
      }
      break;
    }
      
    case MYSQL_TYPE_DATE: {
      bind.buffer_type = MYSQL_TYPE_STRING;
      bind.buffer = malloc(11);  // YYYY-MM-DD + null terminator
      if(!bind.buffer) {
        free(bind.is_null);
        throw std::runtime_error("Failed to allocate memory for DATE buffer");
      }
      bind.buffer_length = 11;
      break;
    }
      
    default:
      free(bind.is_null);
      throw std::runtime_error("Buffer type is not supported");
      
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
  if (!dbData->hasMore) {
    return oatpp::Void(nullptr);
  }

  auto dispatcher = static_cast<const data::mapping::type::__class::Collection::PolymorphicDispatcher*>(type->polymorphicDispatcher);
  auto collection = dispatcher->createObject();

  const Type* itemType = dispatcher->getItemType();

  for(v_int32 i = 0; i < dbData->colCount; i ++) {
    mapping::Deserializer::InData inData(&dbData->bindResults[i], dbData->typeResolver);
    dispatcher->addItem(collection, _this->m_deserializer.deserialize(inData, itemType));
  }

  // Move to next row
  int fetchResult = mysql_stmt_fetch(dbData->stmt);
  if (fetchResult == 0) {
    dbData->hasMore = true;
  } else if (fetchResult == MYSQL_NO_DATA) {
    dbData->hasMore = false;
  } else {
    OATPP_LOGE("ResultMapper", "Error fetching next row: %s", mysql_stmt_error(dbData->stmt));
    dbData->isSuccess = false;
    dbData->hasMore = false;
  }

  return collection;
}

oatpp::Void ResultMapper::readOneRowAsMap(ResultMapper* _this, ResultData* dbData, const Type* type) {
  if (!dbData->hasMore) {
    return oatpp::Void(nullptr);
  }

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

  // Move to next row
  int fetchResult = mysql_stmt_fetch(dbData->stmt);
  if (fetchResult == 0) {
    dbData->hasMore = true;
  } else if (fetchResult == MYSQL_NO_DATA) {
    dbData->hasMore = false;
  } else {
    OATPP_LOGE("ResultMapper", "Error fetching next row: %s", mysql_stmt_error(dbData->stmt));
    dbData->isSuccess = false;
    dbData->hasMore = false;
  }

  return map;
}

oatpp::Void ResultMapper::readOneRowAsObject(ResultMapper* _this, ResultData* dbData, const Type* type) {
  if (!dbData->hasMore) {
    return oatpp::Void(nullptr);
  }

  auto dispatcher = static_cast<const oatpp::data::mapping::type::__class::AbstractObject::PolymorphicDispatcher*>(type->polymorphicDispatcher);
  auto object = dispatcher->createObject();
  const auto& fieldsMap = dispatcher->getProperties()->getMap();

  // Initialize all Int32 fields to 0 to prevent null pointer issues
  for (const auto& pair : fieldsMap) {
    auto property = pair.second;
    if (property && property->type->classId.id == oatpp::data::mapping::type::__class::Int32::CLASS_ID.id) {
      property->set(static_cast<oatpp::BaseObject*>(object.get()), oatpp::Int32(0));
    }
  }

  // Special case: If there's only one column and it's a RETURNING clause result,
  // try to map it to the matching property type
  if (dbData->colCount == 1) {
    auto& bind = dbData->bindResults[0];
    
    if (!*bind.is_null) {
      oatpp::String fieldName = dbData->colNames[0];
      OATPP_LOGD("ResultMapper", "Column name: %s", fieldName->c_str());
      
      auto it = fieldsMap.find(*fieldName);
      
      // For RETURNING clauses, try to match with 'id' property if exact match not found
      if (it == fieldsMap.end()) {
        OATPP_LOGD("ResultMapper", "Column name not found, trying 'id'");
        it = fieldsMap.find("id");  // Common case for RETURNING id
      }

      if (it != fieldsMap.end()) {
        auto property = it->second;
        OATPP_LOGD("ResultMapper", "Found property: %s", it->first.c_str());
        
        if (property) {
          if (property->type == oatpp::data::mapping::type::__class::Boolean::getType()) {
            if (bind.buffer_type == MYSQL_TYPE_TINY) {
              signed char value = *static_cast<signed char*>(bind.buffer);
              OATPP_LOGD("ResultMapper", "Mapping single column value %d to Boolean property %s", (int)value, it->first.c_str());
              property->set(static_cast<oatpp::BaseObject*>(object.get()), oatpp::Boolean(value != 0));
            }
          } else if (property->type->classId.id == oatpp::data::mapping::type::__class::Int32::CLASS_ID.id) {
            int value = 0;
            bool handled = false;

            switch (bind.buffer_type) {
              case MYSQL_TYPE_TINY:
                value = *static_cast<signed char*>(bind.buffer);
                handled = true;
                break;
              case MYSQL_TYPE_SHORT:
              case MYSQL_TYPE_YEAR:
                value = *static_cast<short*>(bind.buffer);
                handled = true;
                break;
              case MYSQL_TYPE_LONG:
              case MYSQL_TYPE_INT24:
                value = *static_cast<int*>(bind.buffer);
                handled = true;
                break;
              case MYSQL_TYPE_LONGLONG:
                value = static_cast<int>(*static_cast<long long*>(bind.buffer));
                handled = true;
                break;
              default:
                OATPP_LOGD("ResultMapper", "Unhandled buffer type: %d", bind.buffer_type);
                handled = false;
                break;
            }

            if (handled) {
              OATPP_LOGD("ResultMapper", "Mapping single column value %d to Int32 property %s", value, it->first.c_str());
              property->set(static_cast<oatpp::BaseObject*>(object.get()), oatpp::Int32(value));
            }
          } else if (property->type == oatpp::data::mapping::type::__class::Int64::getType() ||
                     property->type == oatpp::data::mapping::type::__class::UInt64::getType()) {
            if (bind.buffer_type == MYSQL_TYPE_LONGLONG) {
              if (*bind.is_null) {
                OATPP_LOGD("ResultMapper", "Setting null int64 value for property %s", fieldName->c_str());
                property->set(static_cast<oatpp::BaseObject*>(object.get()), nullptr);
              } else if (bind.is_unsigned) {
                uint64_t value = *static_cast<uint64_t*>(bind.buffer);
                OATPP_LOGD("ResultMapper", "Setting unsigned int64 value %llu for property %s", value, fieldName->c_str());
                if (property->type == oatpp::data::mapping::type::__class::UInt64::getType()) {
                  property->set(static_cast<oatpp::BaseObject*>(object.get()), oatpp::UInt64(value));
                } else {
                  property->set(static_cast<oatpp::BaseObject*>(object.get()), oatpp::Int64(static_cast<int64_t>(value)));
                }
              } else {
                int64_t value = *static_cast<int64_t*>(bind.buffer);
                OATPP_LOGD("ResultMapper", "Setting signed int64 value %lld for property %s", value, fieldName->c_str());
                if (property->type == oatpp::data::mapping::type::__class::UInt64::getType()) {
                  property->set(static_cast<oatpp::BaseObject*>(object.get()), oatpp::UInt64(static_cast<uint64_t>(value)));
                } else {
                  property->set(static_cast<oatpp::BaseObject*>(object.get()), oatpp::Int64(value));
                }
              }
            } else {
              mapping::Deserializer::InData inData(&bind, dbData->typeResolver);
              property->set(static_cast<oatpp::BaseObject*>(object.get()), _this->m_deserializer.deserialize(inData, property->type));
            }
          } else if (property->type == oatpp::data::mapping::type::__class::Boolean::getType()) {
            if (bind.buffer_type == MYSQL_TYPE_TINY) {
              if (*bind.is_null) {
                OATPP_LOGD("ResultMapper", "Setting null boolean value for property %s", fieldName->c_str());
                property->set(static_cast<oatpp::BaseObject*>(object.get()), nullptr);
              } else {
                signed char value = *static_cast<signed char*>(bind.buffer);
                OATPP_LOGD("ResultMapper", "Setting boolean value %d for property %s", (int)value, fieldName->c_str());
                property->set(static_cast<oatpp::BaseObject*>(object.get()), oatpp::Boolean(value != 0));
              }
            } else {
              mapping::Deserializer::InData inData(&bind, dbData->typeResolver);
              property->set(static_cast<oatpp::BaseObject*>(object.get()), _this->m_deserializer.deserialize(inData, property->type));
            }
          } else {
            mapping::Deserializer::InData inData(&bind, dbData->typeResolver);
            property->set(static_cast<oatpp::BaseObject*>(object.get()), _this->m_deserializer.deserialize(inData, property->type));
          }
        } else {
          OATPP_LOGD("ResultMapper", "Property is null");
        }
      } else {
        OATPP_LOGD("ResultMapper", "No matching property found");
      }
    } else {
      OATPP_LOGD("ResultMapper", "Column value is null");
    }
  } else {
    // Normal case: Map all columns to properties by name
    for (unsigned int i = 0; i < dbData->colCount; i++) {
      auto& bind = dbData->bindResults[i];
      
      oatpp::String fieldName = dbData->colNames[i];
      auto it = fieldsMap.find(*fieldName);
      
      // For RETURNING clauses, try to match with 'id' property if exact match not found
      if (it == fieldsMap.end()) {
        OATPP_LOGD("ResultMapper", "Column name not found, trying 'id'");
        it = fieldsMap.find("id");  // Common case for RETURNING id
      }

      if (it != fieldsMap.end()) {
        auto property = it->second;
        OATPP_LOGD("ResultMapper", "Found property: %s", it->first.c_str());
        
        if (property) {
          if (property->type == oatpp::data::mapping::type::__class::Boolean::getType()) {
            if (bind.buffer_type == MYSQL_TYPE_TINY) {
              if (*bind.is_null) {
                OATPP_LOGD("ResultMapper", "Setting null boolean value for property %s", fieldName->c_str());
                property->set(static_cast<oatpp::BaseObject*>(object.get()), nullptr);
              } else {
                signed char value = *static_cast<signed char*>(bind.buffer);
                OATPP_LOGD("ResultMapper", "Setting boolean value %d for property %s", (int)value, fieldName->c_str());
                property->set(static_cast<oatpp::BaseObject*>(object.get()), oatpp::Boolean(value != 0));
              }
            } else {
              mapping::Deserializer::InData inData(&bind, dbData->typeResolver);
              property->set(static_cast<oatpp::BaseObject*>(object.get()), _this->m_deserializer.deserialize(inData, property->type));
            }
          } else if (property->type == oatpp::data::mapping::type::__class::Int64::getType() ||
                     property->type == oatpp::data::mapping::type::__class::UInt64::getType()) {
            if (bind.buffer_type == MYSQL_TYPE_LONGLONG) {
              if (*bind.is_null) {
                OATPP_LOGD("ResultMapper", "Setting null int64 value for property %s", fieldName->c_str());
                property->set(static_cast<oatpp::BaseObject*>(object.get()), nullptr);
              } else if (bind.is_unsigned) {
                uint64_t value = *static_cast<uint64_t*>(bind.buffer);
                OATPP_LOGD("ResultMapper", "Setting unsigned int64 value %llu for property %s", value, fieldName->c_str());
                if (property->type == oatpp::data::mapping::type::__class::UInt64::getType()) {
                  property->set(static_cast<oatpp::BaseObject*>(object.get()), oatpp::UInt64(value));
                } else {
                  property->set(static_cast<oatpp::BaseObject*>(object.get()), oatpp::Int64(static_cast<int64_t>(value)));
                }
              } else {
                int64_t value = *static_cast<int64_t*>(bind.buffer);
                OATPP_LOGD("ResultMapper", "Setting signed int64 value %lld for property %s", value, fieldName->c_str());
                if (property->type == oatpp::data::mapping::type::__class::UInt64::getType()) {
                  property->set(static_cast<oatpp::BaseObject*>(object.get()), oatpp::UInt64(static_cast<uint64_t>(value)));
                } else {
                  property->set(static_cast<oatpp::BaseObject*>(object.get()), oatpp::Int64(value));
                }
              }
            } else {
              mapping::Deserializer::InData inData(&bind, dbData->typeResolver);
              property->set(static_cast<oatpp::BaseObject*>(object.get()), _this->m_deserializer.deserialize(inData, property->type));
            }
          } else {
            mapping::Deserializer::InData inData(&bind, dbData->typeResolver);
            property->set(static_cast<oatpp::BaseObject*>(object.get()), _this->m_deserializer.deserialize(inData, property->type));
          }
        } else {
          OATPP_LOGD("ResultMapper", "Property is null");
        }
      } else {
        OATPP_LOGD("ResultMapper", "No matching property found");
      }
    }
  }

  // Move to next row
  int fetchResult = mysql_stmt_fetch(dbData->stmt);
  if (fetchResult == 0) {
    dbData->hasMore = true;
  } else if (fetchResult == MYSQL_NO_DATA) {
    dbData->hasMore = false;
  } else {
    OATPP_LOGE("ResultMapper", "Error fetching next row: %s", mysql_stmt_error(dbData->stmt));
    dbData->isSuccess = false;
    dbData->hasMore = false;
  }

  return object;
}

oatpp::Void ResultMapper::readRowsAsCollection(ResultMapper* _this, ResultData* dbData, const Type* type, v_int64 count) {
  auto dispatcher = static_cast<const oatpp::data::mapping::type::__class::Collection::PolymorphicDispatcher*>(type->polymorphicDispatcher);
  oatpp::Void collection = dispatcher->createObject();
  
  const Type* itemType = dispatcher->getItemType();
  
  if (count == -1) {
    while (dbData->hasMore) {
      oatpp::Void item = _this->readOneRowAsObject(_this, dbData, itemType);
      if (item) {
        OATPP_LOGD("ResultMapper", "Adding item to collection at index %d", dbData->rowIndex);
        dispatcher->addItem(collection, item);
      }
      ++dbData->rowIndex;
    }
  } else {
    v_int64 itemsLeft = count;
    while (itemsLeft > 0 && dbData->hasMore) {
      oatpp::Void item = _this->readOneRowAsObject(_this, dbData, itemType);
      if (item) {
        OATPP_LOGD("ResultMapper", "Adding item to collection at index %d", dbData->rowIndex);
        dispatcher->addItem(collection, item);
      }
      ++dbData->rowIndex;
      --itemsLeft;
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

  if (!dbData->isSuccess) {
    return nullptr;
  }

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

  throw std::runtime_error("[oatpp::mariadb::mapping::ResultMapper::readOneRow]: Error. "
                          "No deserializer found for type '" + std::string(type->classId.name) + "'");

}

oatpp::Void ResultMapper::readRows(ResultData* dbData, const Type* type, v_int64 count) {

  auto id = type->classId.id;
  if(id >= m_readRowsMethods.size() || !m_readRowsMethods[id]) {
    throw std::runtime_error("[oatpp::mariadb::mapping::ResultMapper::readRows]: "
                           "Error. Invalid result container type. "
                           "Allowed types are oatpp::Vector, oatpp::List, oatpp::UnorderedSet");
  }

  return m_readRowsMethods[id](this, dbData, type, count);

}

}}}
