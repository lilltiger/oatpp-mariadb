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
  // free memory allocated for is_null pointers
  for(auto& bindParam : m_bindParams) {
    if(bindParam.is_null) {
      free(bindParam.is_null);
      bindParam.is_null = nullptr;
    }
  }
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
  m_bindParams[paramIndex] = bind;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Serializer functions

void Serializer::serializeString(const Serializer* _this, MYSQL_STMT* stmt, v_uint32 paramIndex, const oatpp::Void& polymorph) {
  MYSQL_BIND bindParam;
  std::memset(&bindParam, 0, sizeof(bindParam));
  bindParam.buffer_type = MYSQL_TYPE_STRING;

  if(polymorph) {
    std::string *buff = static_cast<std::string*>(polymorph.get());

    bindParam.buffer = static_cast<void*>(const_cast<char*>(buff->c_str()));
    bindParam.buffer_length = buff->size();
    bindParam.is_null = 0;

    OATPP_LOGD("Serializer::serializeString()", "value='%s'", buff->c_str());
  } else {
    bindParam.is_null = static_cast<my_bool*>(malloc(sizeof(my_bool)));
    *bindParam.is_null = 1;
    OATPP_LOGD("Serializer::serializeString()", "null");
  }

  _this->setBindParam(bindParam, paramIndex);
}

void Serializer::serializeInt8(const Serializer* _this, MYSQL_STMT* stmt, v_uint32 paramIndex, const oatpp::Void& polymorph) {
  MYSQL_BIND bindParam;
  std::memset(&bindParam, 0, sizeof(bindParam));
  bindParam.buffer_type = MYSQL_TYPE_TINY;

  if(polymorph) {
    auto v = polymorph.cast<oatpp::Int8>();

    bindParam.buffer = v.get();
    bindParam.buffer_length = 0;
    bindParam.is_null = 0;
  } else {
    bindParam.is_null = static_cast<my_bool*>(malloc(sizeof(my_bool)));
    *bindParam.is_null = 1;
  }

  _this->setBindParam(bindParam, paramIndex);
}

void Serializer::serializeUInt8(const Serializer* _this, MYSQL_STMT* stmt, v_uint32 paramIndex, const oatpp::Void& polymorph) {
  MYSQL_BIND bindParam;
  std::memset(&bindParam, 0, sizeof(bindParam));
  bindParam.buffer_type = MYSQL_TYPE_TINY;

  if(polymorph) {
    auto v = polymorph.cast<oatpp::UInt8>();

    bindParam.buffer = v.get();
    bindParam.buffer_length = 0;
    bindParam.is_null = 0;
  } else {
    bindParam.is_null = static_cast<my_bool*>(malloc(sizeof(my_bool)));
    *bindParam.is_null = 1;
  }

  _this->setBindParam(bindParam, paramIndex);
}

void Serializer::serializeInt16(const Serializer* _this, MYSQL_STMT* stmt, v_uint32 paramIndex, const oatpp::Void& polymorph) {
  MYSQL_BIND bindParam;
  std::memset(&bindParam, 0, sizeof(bindParam));
  bindParam.buffer_type = MYSQL_TYPE_SHORT;

  if(polymorph) {
    auto v = polymorph.cast<oatpp::Int16>();

    bindParam.buffer = v.get();
    bindParam.buffer_length = 0;
    bindParam.is_null = 0;
  } else {
    bindParam.is_null = static_cast<my_bool*>(malloc(sizeof(my_bool)));
    *bindParam.is_null = 1;
  }

  _this->setBindParam(bindParam, paramIndex);
}

void Serializer::serializeUInt16(const Serializer* _this, MYSQL_STMT* stmt, v_uint32 paramIndex, const oatpp::Void& polymorph) {
  MYSQL_BIND bindParam;
  std::memset(&bindParam, 0, sizeof(bindParam));
  bindParam.buffer_type = MYSQL_TYPE_SHORT;

  if(polymorph) {
    auto v = polymorph.cast<oatpp::UInt16>();

    bindParam.buffer = v.get();
    bindParam.buffer_length = 0;
    bindParam.is_null = 0;
  } else {
    bindParam.is_null = static_cast<my_bool*>(malloc(sizeof(my_bool)));
    *bindParam.is_null = 1;
  }

  _this->setBindParam(bindParam, paramIndex);
}

void Serializer::serializeInt32(const Serializer* _this, MYSQL_STMT* stmt, v_uint32 paramIndex, const oatpp::Void& polymorph) {
  MYSQL_BIND bindParam;
  std::memset(&bindParam, 0, sizeof(bindParam));
  bindParam.buffer_type = MYSQL_TYPE_LONG;

  if(polymorph) {
    auto v = polymorph.cast<oatpp::Int32>();

    bindParam.buffer = v.get();
    bindParam.buffer_length = 0;
    bindParam.is_null = 0;
  } else {
    bindParam.is_null = static_cast<my_bool*>(malloc(sizeof(my_bool)));
    *bindParam.is_null = 1;
  }

  _this->setBindParam(bindParam, paramIndex);
}

void Serializer::serializeUInt32(const Serializer* _this, MYSQL_STMT* stmt, v_uint32 paramIndex, const oatpp::Void& polymorph) {
  MYSQL_BIND bindParam;
  std::memset(&bindParam, 0, sizeof(bindParam));
  bindParam.buffer_type = MYSQL_TYPE_LONG;

  if(polymorph) {
    auto v = polymorph.cast<oatpp::UInt32>();

    bindParam.buffer = v.get();
    bindParam.buffer_length = 0;
    bindParam.is_null = 0;
  } else {
    bindParam.is_null = static_cast<my_bool*>(malloc(sizeof(my_bool)));
    *bindParam.is_null = 1;
  }

  _this->setBindParam(bindParam, paramIndex);
}

void Serializer::serializeInt64(const Serializer* _this, MYSQL_STMT* stmt, v_uint32 paramIndex, const oatpp::Void& polymorph) {
  MYSQL_BIND bindParam;
  std::memset(&bindParam, 0, sizeof(bindParam));
  bindParam.buffer_type = MYSQL_TYPE_LONGLONG;

  if(polymorph) {
    auto v = polymorph.cast<oatpp::Int64>();

    bindParam.buffer = v.get();
    bindParam.buffer_length = 0;
    bindParam.is_null = 0;
  } else {
    bindParam.is_null = static_cast<my_bool*>(malloc(sizeof(my_bool)));
    *bindParam.is_null = 1;
  }

  _this->setBindParam(bindParam, paramIndex);
}

void Serializer::serializeUInt64(const Serializer* _this, MYSQL_STMT* stmt, v_uint32 paramIndex, const oatpp::Void& polymorph) {
  MYSQL_BIND bindParam;
  std::memset(&bindParam, 0, sizeof(bindParam));
  bindParam.buffer_type = MYSQL_TYPE_LONGLONG;

  if(polymorph) {
    auto v = polymorph.cast<oatpp::UInt64>();

    bindParam.buffer = v.get();
    bindParam.buffer_length = 0;
    bindParam.is_null = 0;
  } else {
    bindParam.is_null = static_cast<my_bool*>(malloc(sizeof(my_bool)));
    *bindParam.is_null = 1;
  }

  _this->setBindParam(bindParam, paramIndex);
}

void Serializer::serializeFloat32(const Serializer* _this, MYSQL_STMT* stmt, v_uint32 paramIndex, const oatpp::Void& polymorph) {
  MYSQL_BIND bindParam;
  std::memset(&bindParam, 0, sizeof(bindParam));
  bindParam.buffer_type = MYSQL_TYPE_FLOAT;

  if(polymorph) {
    auto v = polymorph.cast<oatpp::Float32>();

    bindParam.buffer = v.get();
    bindParam.buffer_length = 0;
    bindParam.is_null = 0;
  } else {
    bindParam.is_null = static_cast<my_bool*>(malloc(sizeof(my_bool)));
    *bindParam.is_null = 1;
  }

  _this->setBindParam(bindParam, paramIndex);
}

void Serializer::serializeFloat64(const Serializer* _this, MYSQL_STMT* stmt, v_uint32 paramIndex, const oatpp::Void& polymorph) {
  MYSQL_BIND bindParam;
  std::memset(&bindParam, 0, sizeof(bindParam));
  bindParam.buffer_type = MYSQL_TYPE_DOUBLE;

  if(polymorph) {
    auto v = polymorph.cast<oatpp::Float64>();

    bindParam.buffer = v.get();
    bindParam.buffer_length = 0;
    bindParam.is_null = 0;
  } else {
    bindParam.is_null = static_cast<my_bool*>(malloc(sizeof(my_bool)));
    *bindParam.is_null = 1;
  }

  _this->setBindParam(bindParam, paramIndex);
}

void Serializer::serializeEnum(const Serializer* _this, MYSQL_STMT* stmt, v_uint32 paramIndex, const oatpp::Void& polymorph) {

  auto polymorphicDispatcher = static_cast<const data::mapping::type::__class::AbstractEnum::PolymorphicDispatcher*>(
    polymorph.getValueType()->polymorphicDispatcher
  );

  data::mapping::type::EnumInterpreterError e = data::mapping::type::EnumInterpreterError::OK;
  const auto& enumInterpretation = polymorphicDispatcher->toInterpretation(polymorph, e);

  if(e == data::mapping::type::EnumInterpreterError::OK) {
    _this->serialize(stmt, paramIndex, enumInterpretation);
    return;
  }

  switch(e) {
    case data::mapping::type::EnumInterpreterError::CONSTRAINT_NOT_NULL:
      throw std::runtime_error("[oatpp::mariadb::mapping::Serializer::serializeEnum()]: Error. Enum constraint violated - 'NotNull'.");
    default:
      throw std::runtime_error("[oatpp::mariadb::mapping::Serializer::serializeEnum()]: Error. Can't serialize Enum.");
  }

}

}}}
