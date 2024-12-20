/***************************************************************************
 *
 * Project         _____    __   ____   _      _
 *                (  _  )  /__\ (_  _)_| |_  _| |_
 *                 )(_)(  /(__)\  )( (_   _)(_   _)
 *                (_____)(__)(__)(__)  |_|    |_|
 *
 *
 * Copyright 2018-present, Leonid Stryzhevskyi <lganzzzo@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ***************************************************************************/

#include "Serializer.hpp"

#if defined(WIN32) || defined(_WIN32)
  #include <winsock2.h>
#else
  #include <arpa/inet.h>
#endif

namespace oatpp { namespace mariadb { namespace mapping {

Serializer::Serializer() {

  m_methods.resize(data::mapping::type::ClassId::getClassCount(), nullptr);

  setSerializerMethod(data::mapping::type::__class::String::CLASS_ID, &Serializer::serializeString);
  setSerializerMethod(data::mapping::type::__class::Any::CLASS_ID, nullptr);
  setSerializerMethod(data::mapping::type::__class::Boolean::CLASS_ID, &Serializer::serializeBoolean);

  setSerializerMethod(data::mapping::type::__class::Int8::CLASS_ID, &Serializer::serializeInt8);
  setSerializerMethod(data::mapping::type::__class::UInt8::CLASS_ID, &Serializer::serializeUInt8);

  setSerializerMethod(data::mapping::type::__class::Int16::CLASS_ID, &Serializer::serializeInt16);
  setSerializerMethod(data::mapping::type::__class::UInt16::CLASS_ID, &Serializer::serializeUInt16);

  setSerializerMethod(data::mapping::type::__class::Int32::CLASS_ID, &Serializer::serializeInt32);
  setSerializerMethod(data::mapping::type::__class::UInt32::CLASS_ID, &Serializer::serializeUInt32);

  setSerializerMethod(data::mapping::type::__class::Int64::CLASS_ID, &Serializer::serializeInt64);
  setSerializerMethod(data::mapping::type::__class::UInt64::CLASS_ID, &Serializer::serializeUInt64);

  setSerializerMethod(data::mapping::type::__class::Float32::CLASS_ID, &Serializer::serializeFloat32);
  setSerializerMethod(data::mapping::type::__class::Float64::CLASS_ID, &Serializer::serializeFloat64);

  setSerializerMethod(data::mapping::type::__class::AbstractObject::CLASS_ID, nullptr);
  setSerializerMethod(data::mapping::type::__class::AbstractEnum::CLASS_ID, &Serializer::serializeEnum);

  setSerializerMethod(data::mapping::type::__class::AbstractVector::CLASS_ID, nullptr);
  setSerializerMethod(data::mapping::type::__class::AbstractList::CLASS_ID, nullptr);
  setSerializerMethod(data::mapping::type::__class::AbstractUnorderedSet::CLASS_ID, nullptr);

  setSerializerMethod(data::mapping::type::__class::AbstractPairList::CLASS_ID, nullptr);
  setSerializerMethod(data::mapping::type::__class::AbstractUnorderedMap::CLASS_ID, nullptr);

}

Serializer::~Serializer() {
  // Clean up all allocated memory
  for(auto& param : m_bindParams) {
    if(param.buffer != nullptr) {
      free(param.buffer);
      param.buffer = nullptr;
    }
    if(param.is_null != nullptr && param.is_null != &param.is_null_value) {
      free(param.is_null);
      param.is_null = nullptr;
    }
    if(param.length != nullptr && param.length != &param.length_value) {
      free(param.length);
      param.length = nullptr;
    }
    if(param.error != nullptr && param.error != &param.error_value) {
      free(param.error);
      param.error = nullptr;
    }
  }
  m_bindParams.clear();
}

void Serializer::setSerializerMethod(const data::mapping::type::ClassId& classId, SerializerMethod method) {
  const v_uint32 id = classId.id;
  if(id >= m_methods.size()) {
    m_methods.resize(id + 1, nullptr);
  }
  m_methods[id] = method;
}

void Serializer::serialize(MYSQL_STMT* stmt, v_uint32 paramIndex, const oatpp::Void& polymorph) const {
  auto id = polymorph.getValueType()->classId.id;
  auto& method = m_methods[id];

  OATPP_LOGD("Serializer::serialize()", "classId=%d, className=%s, paramIndex=%d, method=%p", 
    id, polymorph.getValueType()->classId.name, paramIndex, method);

  if(method) {
    (*method)(this, stmt, paramIndex, polymorph);
  } else {
    throw std::runtime_error("[oatpp::mariadb::mapping::Serializer::serialize()]: "
                             "Error. No serialize method for type '" + std::string(polymorph.getValueType()->classId.name) +
                             "'");
  }
}

std::vector<MYSQL_BIND>& Serializer::getBindParams() const {
  return m_bindParams;
}

void Serializer::setBindParam(MYSQL_BIND& bind, v_uint32 paramIndex) const {
  if (paramIndex >= m_bindParams.size()) {
    m_bindParams.resize(paramIndex + 1);
  }
  // Zero out the target bind structure before copying
  std::memset(&m_bindParams[paramIndex], 0, sizeof(MYSQL_BIND));
  // Copy the bind parameter
  m_bindParams[paramIndex] = bind;
}

void Serializer::bindParameters(MYSQL_STMT* stmt) const {
  if (!stmt) {
    OATPP_LOGE("Serializer", "Error: MySQL statement is null");
    throw std::runtime_error("MySQL statement is null");
  }
  
  if (!m_bindParams.empty()) {
    if(mysql_stmt_bind_param(stmt, m_bindParams.data())) {
      auto error = mysql_stmt_error(stmt);
      OATPP_LOGE("Serializer", "Failed to bind parameters: %s", error);
      throw std::runtime_error(error);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Serializer functions

void Serializer::serializeString(const Serializer* _this, MYSQL_STMT* stmt, v_uint32 paramIndex, const oatpp::Void& polymorph) {
  OATPP_LOGD("Serializer", "Serializing String value for paramIndex=%d", paramIndex);
  
  if (!stmt) {
    OATPP_LOGE("Serializer", "Error: MySQL statement is null");
    throw std::runtime_error("MySQL statement is null");
  }
  
  if(paramIndex >= _this->m_bindParams.size()) {
    OATPP_LOGD("Serializer", "Resizing bind params array from %d to %d", _this->m_bindParams.size(), paramIndex + 1);
    _this->m_bindParams.resize(paramIndex + 1);
  }
  
  auto& bind = _this->m_bindParams[paramIndex];
  std::memset(&bind, 0, sizeof(MYSQL_BIND));
  
  bind.is_null = (my_bool*)malloc(sizeof(my_bool));
  if(!bind.is_null) {
    OATPP_LOGE("Serializer", "Failed to allocate memory for is_null indicator");
    throw std::runtime_error("Failed to allocate memory for is_null indicator");
  }
  
  bind.length = (unsigned long*)malloc(sizeof(unsigned long));
  if(!bind.length) {
    free(bind.is_null);
    OATPP_LOGE("Serializer", "Failed to allocate memory for length indicator");
    throw std::runtime_error("Failed to allocate memory for length indicator");
  }
  
  if(polymorph) {
    auto str = polymorph.cast<oatpp::String>();
    if(str) {
      const char* cstr = str->c_str();
      std::size_t len = str->length();
      
      // Always allocate buffer, even for empty strings
      bind.buffer = malloc(len > 0 ? len : 1);
      if(!bind.buffer) {
        free(bind.is_null);
        free(bind.length);
        OATPP_LOGE("Serializer", "Failed to allocate memory for string value");
        throw std::runtime_error("Failed to allocate memory for string value");
      }
      
      if (len > 0) {
        std::memcpy(bind.buffer, cstr, len);
      }
      
      bind.buffer_length = len;
      *bind.length = len;
      *bind.is_null = 0;

      // Check if this is a binary string by looking for non-printable characters
      bool isBinary = false;
      for(size_t i = 0; i < len; i++) {
        unsigned char c = static_cast<unsigned char>(cstr[i]);
        if(c > 127 || (c < 32 && c != '\t' && c != '\n' && c != '\r')) {
          isBinary = true;
          break;
        }
      }

      // Use BLOB type for binary data, STRING for text
      bind.buffer_type = isBinary ? MYSQL_TYPE_BLOB : MYSQL_TYPE_STRING;
      OATPP_LOGD("Serializer", "Using %s type for field, length=%lu, data=%s", 
                 isBinary ? "BLOB" : "STRING", 
                 *bind.length,
                 isBinary ? "<binary>" : (len > 0 ? cstr : "<empty>"));
    } else {
      bind.buffer = nullptr;
      bind.buffer_length = 0;
      *bind.length = 0;
      *bind.is_null = 1;
      bind.buffer_type = MYSQL_TYPE_STRING;
      OATPP_LOGD("Serializer", "String value is null");
    }
  } else {
    bind.buffer = nullptr;
    bind.buffer_length = 0;
    *bind.length = 0;
    *bind.is_null = 1;
    bind.buffer_type = MYSQL_TYPE_STRING;
  }
}

void Serializer::serializeBoolean(const Serializer* _this, MYSQL_STMT* stmt, v_uint32 paramIndex, const oatpp::Void& polymorph) {
  OATPP_LOGD("Serializer", "Serializing Boolean value for paramIndex=%d", paramIndex);
  
  if (!stmt) {
    OATPP_LOGE("Serializer", "Error: MySQL statement is null");
    throw std::runtime_error("MySQL statement is null");
  }
  
  if(paramIndex >= _this->m_bindParams.size()) {
    OATPP_LOGD("Serializer", "Resizing bind params array from %d to %d", _this->m_bindParams.size(), paramIndex + 1);
    _this->m_bindParams.resize(paramIndex + 1);
  }
  
  auto& bind = _this->m_bindParams[paramIndex];
  std::memset(&bind, 0, sizeof(MYSQL_BIND));
  
  bind.is_null = (my_bool*)malloc(sizeof(my_bool));
  if(!bind.is_null) {
    OATPP_LOGE("Serializer", "Failed to allocate memory for is_null indicator");
    throw std::runtime_error("Failed to allocate memory for is_null indicator");
  }
  
  bind.buffer = malloc(sizeof(signed char));
  if(!bind.buffer) {
    free(bind.is_null);
    OATPP_LOGE("Serializer", "Failed to allocate memory for Boolean value");
    throw std::runtime_error("Failed to allocate memory for Boolean value");
  }
  
  bind.buffer_type = MYSQL_TYPE_TINY;
  bind.is_unsigned = false;  // Use signed char for boolean
  bind.buffer_length = sizeof(signed char);
  
  if(polymorph) {
    auto value = polymorph.cast<oatpp::Boolean>();
    if(value.getValueType() != nullptr) {  // Check if value is not null
      *static_cast<signed char*>(bind.buffer) = value.getValue(false) ? 1 : 0;
      *bind.is_null = 0;
      OATPP_LOGD("Serializer", "Serializing boolean value: %d", (int)*static_cast<signed char*>(bind.buffer));
    } else {
      *bind.is_null = 1;
      *static_cast<signed char*>(bind.buffer) = 0;
      OATPP_LOGD("Serializer", "Serializing null boolean value (value is null)");
    }
  } else {
    *bind.is_null = 1;
    *static_cast<signed char*>(bind.buffer) = 0;
    OATPP_LOGD("Serializer", "Serializing null boolean value (polymorph is null)");
  }
}

void Serializer::serializeInt8(const Serializer* _this, MYSQL_STMT* stmt, v_uint32 paramIndex, const oatpp::Void& polymorph) {
  if(paramIndex >= _this->m_bindParams.size()) {
    _this->m_bindParams.resize(paramIndex + 1);
  }
  
  auto& bind = _this->m_bindParams[paramIndex];
  std::memset(&bind, 0, sizeof(MYSQL_BIND));
  bind.buffer_type = MYSQL_TYPE_TINY;
  
  if(polymorph) {
    auto value = polymorph.cast<oatpp::Int8>();
    if(value) {
      bind.buffer = malloc(sizeof(int8_t));
      if(!bind.buffer) {
        throw std::runtime_error("Failed to allocate memory for Int8 value");
      }
      *static_cast<int8_t*>(bind.buffer) = value;
      bind.buffer_length = sizeof(int8_t);
      bind.is_null_value = 0;
    } else {
      bind.is_null_value = 1;
    }
  } else {
    bind.is_null_value = 1;
  }
  
  bind.is_null = &bind.is_null_value;
}

void Serializer::serializeUInt8(const Serializer* _this, MYSQL_STMT* stmt, v_uint32 paramIndex, const oatpp::Void& polymorph) {
  OATPP_LOGD("Serializer", "Serializing UInt8 value for paramIndex=%d", paramIndex);
  
  if (!stmt) {
    OATPP_LOGE("Serializer", "Error: MySQL statement is null");
    throw std::runtime_error("MySQL statement is null");
  }
  
  if(paramIndex >= _this->m_bindParams.size()) {
    OATPP_LOGD("Serializer", "Resizing bind params array from %d to %d", _this->m_bindParams.size(), paramIndex + 1);
    _this->m_bindParams.resize(paramIndex + 1);
  }
  
  auto& bind = _this->m_bindParams[paramIndex];
  std::memset(&bind, 0, sizeof(MYSQL_BIND));
  
  // Allocate buffer for the value
  bind.buffer = malloc(sizeof(uint8_t));
  if(!bind.buffer) {
    OATPP_LOGE("Serializer", "Failed to allocate memory for UInt8 value");
    throw std::runtime_error("Failed to allocate memory for UInt8 value");
  }
  std::memset(bind.buffer, 0, sizeof(uint8_t));
  
  // Allocate is_null indicator
  bind.is_null = (my_bool*)malloc(sizeof(my_bool));
  if(!bind.is_null) {
    free(bind.buffer);
    OATPP_LOGE("Serializer", "Failed to allocate memory for is_null indicator");
    throw std::runtime_error("Failed to allocate memory for is_null indicator");
  }
  
  bind.buffer_type = MYSQL_TYPE_TINY;
  bind.is_unsigned = 1;  // Unsigned integer
  bind.buffer_length = sizeof(uint8_t);
  
  if(polymorph) {
    auto value = polymorph.cast<oatpp::UInt8>();
    if(value) {
      *static_cast<uint8_t*>(bind.buffer) = static_cast<uint8_t>(*value);
      *bind.is_null = 0;
      OATPP_LOGD("Serializer", "UInt8 value set: %u", *static_cast<uint8_t*>(bind.buffer));
    } else {
      *bind.is_null = 1;
      OATPP_LOGD("Serializer", "UInt8 value is null");
    }
  } else {
    *bind.is_null = 1;
    OATPP_LOGD("Serializer", "UInt8 value is null (polymorph is null)");
  }
}

void Serializer::serializeInt16(const Serializer* _this, MYSQL_STMT* stmt, v_uint32 paramIndex, const oatpp::Void& polymorph) {
  if(paramIndex >= _this->m_bindParams.size()) {
    _this->m_bindParams.resize(paramIndex + 1);
  }
  
  auto& bind = _this->m_bindParams[paramIndex];
  std::memset(&bind, 0, sizeof(MYSQL_BIND));
  bind.buffer_type = MYSQL_TYPE_SHORT;
  
  if(polymorph) {
    auto value = polymorph.cast<oatpp::Int16>();
    if(value) {
      bind.buffer = malloc(sizeof(int16_t));
      if(!bind.buffer) {
        throw std::runtime_error("Failed to allocate memory for Int16 value");
      }
      *static_cast<int16_t*>(bind.buffer) = value;
      bind.buffer_length = sizeof(int16_t);
      bind.is_null_value = 0;
    } else {
      bind.is_null_value = 1;
    }
  } else {
    bind.is_null_value = 1;
  }
  
  bind.is_null = &bind.is_null_value;
}

void Serializer::serializeUInt16(const Serializer* _this, MYSQL_STMT* stmt, v_uint32 paramIndex, const oatpp::Void& polymorph) {
  if(paramIndex >= _this->m_bindParams.size()) {
    _this->m_bindParams.resize(paramIndex + 1);
  }
  
  auto& bind = _this->m_bindParams[paramIndex];
  std::memset(&bind, 0, sizeof(MYSQL_BIND));
  bind.buffer_type = MYSQL_TYPE_SHORT;
  bind.is_unsigned = true;
  
  if(polymorph) {
    auto value = polymorph.cast<oatpp::UInt16>();
    if(value) {
      bind.buffer = malloc(sizeof(uint16_t));
      if(!bind.buffer) {
        throw std::runtime_error("Failed to allocate memory for UInt16 value");
      }
      *static_cast<uint16_t*>(bind.buffer) = value;
      bind.buffer_length = sizeof(uint16_t);
      bind.is_null_value = 0;
    } else {
      bind.is_null_value = 1;
    }
  } else {
    bind.is_null_value = 1;
  }
  
  bind.is_null = &bind.is_null_value;
}

void Serializer::serializeInt32(const Serializer* _this, MYSQL_STMT* stmt, v_uint32 paramIndex, const oatpp::Void& polymorph) {
  OATPP_LOGD("Serializer", "Serializing Int32 value for paramIndex=%d", paramIndex);
  
  if (!stmt) {
    OATPP_LOGE("Serializer", "Error: MySQL statement is null");
    throw std::runtime_error("MySQL statement is null");
  }
  
  if(paramIndex >= _this->m_bindParams.size()) {
    OATPP_LOGD("Serializer", "Resizing bind params array from %d to %d", _this->m_bindParams.size(), paramIndex + 1);
    _this->m_bindParams.resize(paramIndex + 1);
  }
  
  auto& bind = _this->m_bindParams[paramIndex];
  std::memset(&bind, 0, sizeof(MYSQL_BIND));
  
  if(polymorph) {
    auto value = polymorph.cast<oatpp::Int32>();
    
    bind.is_null = (my_bool*)malloc(sizeof(my_bool));
    if(!bind.is_null) {
      throw std::runtime_error("Failed to allocate memory for is_null indicator");
    }
    
    bind.length = (unsigned long*)malloc(sizeof(unsigned long));
    if(!bind.length) {
      free(bind.is_null);
      throw std::runtime_error("Failed to allocate memory for length indicator");
    }
    
    if(value) {
      bind.buffer_type = MYSQL_TYPE_LONG;
      bind.buffer = malloc(sizeof(int32_t));
      if(!bind.buffer) {
        free(bind.is_null);
        free(bind.length);
        throw std::runtime_error("Failed to allocate memory for Int32 value");
      }
      *static_cast<int32_t*>(bind.buffer) = static_cast<int32_t>(value);
      bind.buffer_length = sizeof(int32_t);
      bind.is_unsigned = 0;
      *bind.is_null = 0;
      *bind.length = sizeof(int32_t);
      
      OATPP_LOGD("Serializer", "Int32 value set: %d, buffer_type=%d, buffer_length=%d", 
                 static_cast<int32_t>(value), bind.buffer_type, bind.buffer_length);
    } else {
      bind.buffer_type = MYSQL_TYPE_NULL;
      *bind.is_null = 1;
      *bind.length = 0;
      bind.buffer = nullptr;
      bind.buffer_length = 0;
      OATPP_LOGD("Serializer", "Int32 value is null");
    }
  } else {
    bind.is_null = (my_bool*)malloc(sizeof(my_bool));
    if(!bind.is_null) {
      throw std::runtime_error("Failed to allocate memory for is_null indicator");
    }
    
    bind.length = (unsigned long*)malloc(sizeof(unsigned long));
    if(!bind.length) {
      free(bind.is_null);
      throw std::runtime_error("Failed to allocate memory for length indicator");
    }
    
    bind.buffer_type = MYSQL_TYPE_NULL;
    *bind.is_null = 1;
    *bind.length = 0;
    bind.buffer = nullptr;
    bind.buffer_length = 0;
    OATPP_LOGD("Serializer", "Int32 value is null (polymorph is null)");
  }
}

void Serializer::serializeUInt32(const Serializer* _this, MYSQL_STMT* stmt, v_uint32 paramIndex, const oatpp::Void& polymorph) {
  if(paramIndex >= _this->m_bindParams.size()) {
    _this->m_bindParams.resize(paramIndex + 1);
  }
  
  auto& bind = _this->m_bindParams[paramIndex];
  std::memset(&bind, 0, sizeof(MYSQL_BIND));
  bind.buffer_type = MYSQL_TYPE_LONG;
  bind.is_unsigned = true;
  
  if(polymorph) {
    auto value = polymorph.cast<oatpp::UInt32>();
    if(value) {
      bind.buffer = malloc(sizeof(uint32_t));
      if(!bind.buffer) {
        throw std::runtime_error("Failed to allocate memory for UInt32 value");
      }
      *static_cast<uint32_t*>(bind.buffer) = value;
      bind.buffer_length = sizeof(uint32_t);
      bind.is_null_value = 0;
    } else {
      bind.is_null_value = 1;
    }
  } else {
    bind.is_null_value = 1;
  }
  
  bind.is_null = &bind.is_null_value;
}

void Serializer::serializeInt64(const Serializer* _this, MYSQL_STMT* stmt, v_uint32 paramIndex, const oatpp::Void& polymorph) {
  OATPP_LOGD("Serializer", "Serializing Int64 value for paramIndex=%d", paramIndex);
  
  if (!stmt) {
    OATPP_LOGE("Serializer", "Error: MySQL statement is null");
    throw std::runtime_error("MySQL statement is null");
  }
  
  if(paramIndex >= _this->m_bindParams.size()) {
    OATPP_LOGD("Serializer", "Resizing bind params array from %d to %d", _this->m_bindParams.size(), paramIndex + 1);
    _this->m_bindParams.resize(paramIndex + 1);
  }
  
  auto& bind = _this->m_bindParams[paramIndex];
  std::memset(&bind, 0, sizeof(MYSQL_BIND));
  bind.buffer_type = MYSQL_TYPE_LONGLONG;
  bind.is_unsigned = 0;  // Signed integer
  bind.length_value = sizeof(int64_t);
  bind.length = &bind.length_value;
  bind.buffer_length = sizeof(int64_t);
  
  // Allocate buffer regardless of value to maintain consistent memory layout
  bind.buffer = malloc(sizeof(int64_t));
  if(!bind.buffer) {
    OATPP_LOGE("Serializer", "Failed to allocate memory for Int64 value");
    throw std::runtime_error("Failed to allocate memory for Int64 value");
  }
  
  // Initialize buffer to 0
  std::memset(bind.buffer, 0, sizeof(int64_t));
  
  if(polymorph) {
    auto value = polymorph.cast<oatpp::Int64>();
    if(value) {
      *static_cast<int64_t*>(bind.buffer) = static_cast<int64_t>(value);
      bind.is_null_value = 0;
      OATPP_LOGD("Serializer", "Int64 value set: %lld", *static_cast<int64_t*>(bind.buffer));
    } else {
      bind.is_null_value = 1;
      OATPP_LOGD("Serializer", "Int64 value is null");
    }
  } else {
    bind.is_null_value = 1;
    OATPP_LOGD("Serializer", "Int64 value is null (polymorph is null)");
  }
  
  bind.is_null = &bind.is_null_value;
  OATPP_LOGD("Serializer", "Int64 binding completed for paramIndex=%d, is_null=%d", paramIndex, bind.is_null_value);
}

void Serializer::serializeUInt64(const Serializer* _this, MYSQL_STMT* stmt, v_uint32 paramIndex, const oatpp::Void& polymorph) {
  OATPP_LOGD("Serializer", "Serializing UInt64 value for paramIndex=%d", paramIndex);
  
  if (!stmt) {
    OATPP_LOGE("Serializer", "Error: MySQL statement is null");
    throw std::runtime_error("MySQL statement is null");
  }
  
  if(paramIndex >= _this->m_bindParams.size()) {
    OATPP_LOGD("Serializer", "Resizing bind params array from %d to %d", _this->m_bindParams.size(), paramIndex + 1);
    _this->m_bindParams.resize(paramIndex + 1);
  }
  
  auto& bind = _this->m_bindParams[paramIndex];
  std::memset(&bind, 0, sizeof(MYSQL_BIND));
  bind.buffer_type = MYSQL_TYPE_LONGLONG;
  bind.is_unsigned = 1;  // Unsigned integer
  bind.length_value = sizeof(uint64_t);
  bind.length = &bind.length_value;
  bind.buffer_length = sizeof(uint64_t);
  
  // Allocate buffer regardless of value to maintain consistent memory layout
  bind.buffer = malloc(sizeof(uint64_t));
  if(!bind.buffer) {
    OATPP_LOGE("Serializer", "Failed to allocate memory for UInt64 value");
    throw std::runtime_error("Failed to allocate memory for UInt64 value");
  }
  
  // Initialize buffer to 0
  std::memset(bind.buffer, 0, sizeof(uint64_t));
  
  if(polymorph) {
    auto value = polymorph.cast<oatpp::UInt64>();
    if(value) {
      *static_cast<uint64_t*>(bind.buffer) = static_cast<uint64_t>(value);
      bind.is_null_value = 0;
      OATPP_LOGD("Serializer", "UInt64 value set: %llu", *static_cast<uint64_t*>(bind.buffer));
    } else {
      bind.is_null_value = 1;
      OATPP_LOGD("Serializer", "UInt64 value is null");
    }
  } else {
    bind.is_null_value = 1;
    OATPP_LOGD("Serializer", "UInt64 value is null (polymorph is null)");
  }
  
  bind.is_null = &bind.is_null_value;
  OATPP_LOGD("Serializer", "UInt64 binding completed for paramIndex=%d, is_null=%d", paramIndex, bind.is_null_value);
}

void Serializer::serializeFloat32(const Serializer* _this, MYSQL_STMT* stmt, v_uint32 paramIndex, const oatpp::Void& polymorph) {
  if(paramIndex >= _this->m_bindParams.size()) {
    _this->m_bindParams.resize(paramIndex + 1);
  }
  
  auto& bind = _this->m_bindParams[paramIndex];
  std::memset(&bind, 0, sizeof(MYSQL_BIND));
  bind.buffer_type = MYSQL_TYPE_FLOAT;
  
  if(polymorph) {
    auto value = polymorph.cast<oatpp::Float32>();
    if(value) {
      bind.buffer = malloc(sizeof(float));
      if(!bind.buffer) {
        throw std::runtime_error("Failed to allocate memory for Float32 value");
      }
      *static_cast<float*>(bind.buffer) = value;
      bind.buffer_length = sizeof(float);
      bind.is_null_value = 0;
    } else {
      bind.is_null_value = 1;
    }
  } else {
    bind.is_null_value = 1;
  }
  
  bind.is_null = &bind.is_null_value;
}

void Serializer::serializeFloat64(const Serializer* _this, MYSQL_STMT* stmt, v_uint32 paramIndex, const oatpp::Void& polymorph) {
  OATPP_LOGD("Serializer", "Serializing Float64 value for paramIndex=%d", paramIndex);
  
  if (!stmt) {
    OATPP_LOGE("Serializer", "Error: MySQL statement is null");
    throw std::runtime_error("MySQL statement is null");
  }
  
  if(paramIndex >= _this->m_bindParams.size()) {
    OATPP_LOGD("Serializer", "Resizing bind params array from %d to %d", _this->m_bindParams.size(), paramIndex + 1);
    _this->m_bindParams.resize(paramIndex + 1);
  }
  
  auto& bind = _this->m_bindParams[paramIndex];
  std::memset(&bind, 0, sizeof(MYSQL_BIND));
  bind.buffer_type = MYSQL_TYPE_DOUBLE;
  bind.is_unsigned = 0;  // Not applicable for doubles, but set for consistency
  bind.length_value = sizeof(double);
  bind.length = &bind.length_value;
  bind.buffer_length = sizeof(double);
  
  // Allocate buffer regardless of value to maintain consistent memory layout
  bind.buffer = malloc(sizeof(double));
  if(!bind.buffer) {
    OATPP_LOGE("Serializer", "Failed to allocate memory for Float64 value");
    throw std::runtime_error("Failed to allocate memory for Float64 value");
  }
  
  // Initialize buffer to 0
  std::memset(bind.buffer, 0, sizeof(double));
  
  if(polymorph) {
    auto value = polymorph.cast<oatpp::Float64>();
    if(value) {
      *static_cast<double*>(bind.buffer) = static_cast<double>(*value);
      bind.is_null_value = 0;
      OATPP_LOGD("Serializer", "Float64 value set: %f", *static_cast<double*>(bind.buffer));
    } else {
      bind.is_null_value = 1;
      OATPP_LOGD("Serializer", "Float64 value is null");
    }
  } else {
    bind.is_null_value = 1;
    OATPP_LOGD("Serializer", "Float64 value is null (polymorph is null)");
  }
  
  bind.is_null = &bind.is_null_value;
  OATPP_LOGD("Serializer", "Float64 binding completed for paramIndex=%d, is_null=%d", paramIndex, bind.is_null_value);
}

void Serializer::serializeEnum(const Serializer* _this, MYSQL_STMT* stmt, v_uint32 paramIndex, const oatpp::Void& polymorph) {
  if(paramIndex >= _this->m_bindParams.size()) {
    _this->m_bindParams.resize(paramIndex + 1);
  }
  
  auto& bind = _this->m_bindParams[paramIndex];
  std::memset(&bind, 0, sizeof(MYSQL_BIND));
  bind.buffer_type = MYSQL_TYPE_STRING;
  
  if(polymorph) {
    auto polymorphicDispatcher = static_cast<const data::mapping::type::__class::AbstractEnum::PolymorphicDispatcher*>(
      polymorph.getValueType()->polymorphicDispatcher
    );
    data::mapping::type::EnumInterpreterError e = data::mapping::type::EnumInterpreterError::OK;
    auto interpretation = polymorphicDispatcher->toInterpretation(polymorph, e);
    if(e == data::mapping::type::EnumInterpreterError::OK) {
      auto str = interpretation.cast<oatpp::String>();
      if(str) {
        const char* cstr = str->c_str();
        std::size_t len = str->length();
        
        bind.buffer = malloc(len + 1);
        if (!bind.buffer) {
          throw std::runtime_error("Failed to allocate memory for enum value");
        }
        std::memcpy(bind.buffer, cstr, len + 1);
        bind.buffer_length = len;
        bind.is_null_value = 0;
        bind.length_value = len;
      } else {
        bind.is_null_value = 1;
      }
    } else {
      bind.is_null_value = 1;
    }
  } else {
    bind.is_null_value = 1;
  }
  
  bind.is_null = &bind.is_null_value;
  bind.length = &bind.length_value;
}

}}}
