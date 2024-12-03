#include "Deserializer.hpp"

namespace oatpp { namespace mariadb { namespace mapping {

Deserializer::InData::InData(MYSQL_BIND* pBind,
                             const std::shared_ptr<const data::mapping::TypeResolver>& pTypeResolver)
{
  bind = pBind;
  typeResolver = pTypeResolver;
  oid = bind->buffer_type;
  isNull = (bind->is_null != nullptr && *bind->is_null == 1);
}

Deserializer::Deserializer() {

  m_methods.resize(data::mapping::type::ClassId::getClassCount(), nullptr);

  setDeserializerMethod(data::mapping::type::__class::String::CLASS_ID, &Deserializer::deserializeString);
  setDeserializerMethod(data::mapping::type::__class::Any::CLASS_ID, &Deserializer::deserializeAny);
  setDeserializerMethod(data::mapping::type::__class::Boolean::CLASS_ID, &Deserializer::deserializeBoolean);

  setDeserializerMethod(data::mapping::type::__class::Int8::CLASS_ID, &Deserializer::deserializeInt<oatpp::Int8>);
  setDeserializerMethod(data::mapping::type::__class::UInt8::CLASS_ID, &Deserializer::deserializeInt<oatpp::UInt8>);

  setDeserializerMethod(data::mapping::type::__class::Int16::CLASS_ID, &Deserializer::deserializeInt<oatpp::Int16>);
  setDeserializerMethod(data::mapping::type::__class::UInt16::CLASS_ID, &Deserializer::deserializeInt<oatpp::UInt16>);

  setDeserializerMethod(data::mapping::type::__class::Int32::CLASS_ID, &Deserializer::deserializeInt<oatpp::Int32>);
  setDeserializerMethod(data::mapping::type::__class::UInt32::CLASS_ID, &Deserializer::deserializeInt<oatpp::UInt32>);

  setDeserializerMethod(data::mapping::type::__class::Int64::CLASS_ID, &Deserializer::deserializeInt<oatpp::Int64>);
  setDeserializerMethod(data::mapping::type::__class::UInt64::CLASS_ID, &Deserializer::deserializeInt<oatpp::UInt64>);

  setDeserializerMethod(data::mapping::type::__class::Float32::CLASS_ID, &Deserializer::deserializeFloat32);
  setDeserializerMethod(data::mapping::type::__class::Float64::CLASS_ID, &Deserializer::deserializeFloat64);

  setDeserializerMethod(data::mapping::type::__class::AbstractObject::CLASS_ID, nullptr);
  setDeserializerMethod(data::mapping::type::__class::AbstractEnum::CLASS_ID, &Deserializer::deserializeEnum);

  setDeserializerMethod(data::mapping::type::__class::AbstractVector::CLASS_ID, nullptr);
  setDeserializerMethod(data::mapping::type::__class::AbstractList::CLASS_ID, nullptr);
  setDeserializerMethod(data::mapping::type::__class::AbstractUnorderedSet::CLASS_ID, nullptr);

  setDeserializerMethod(data::mapping::type::__class::AbstractPairList::CLASS_ID, nullptr);
  setDeserializerMethod(data::mapping::type::__class::AbstractUnorderedMap::CLASS_ID, nullptr);

}

void Deserializer::setDeserializerMethod(const data::mapping::type::ClassId& classId, DeserializerMethod method) {
  const v_uint32 id = classId.id;
  if(id >= m_methods.size()) {
    m_methods.resize(id + 1, nullptr);
  }
  m_methods[id] = method;
}

oatpp::Void Deserializer::deserialize(const InData& data, const Type* type) const {

  // OATPP_LOGD("Deserializer::deserialize()", "type=%s, oid=%d, isNull=%d", type->classId.name, data.oid, data.isNull);

  auto id = type->classId.id;
  auto& method = m_methods[id];

  if(method) {
    return (*method)(this, data, type);
  }

  auto* interpretation = type->findInterpretation(data.typeResolver->getEnabledInterpretations());
  if(interpretation) {
    return interpretation->fromInterpretation(deserialize(data, interpretation->getInterpretationType()));
  }

  throw std::runtime_error("[oatpp::mariadb::mapping::Deserializer::deserialize()]: "
                           "Error. No deserialize method for type '" + std::string(type->classId.name) + "'");

}

v_int64 Deserializer::deInt(const InData& data) {
  v_int64 value;

  switch(data.oid) {
    case MYSQL_TYPE_BIT: {
      value = *(uint64_t*) data.bind->buffer;
      std::memset(data.bind->buffer, 0, sizeof(uint64_t));
      return value;
    }
    case MYSQL_TYPE_TINY: {
      value = *(int8_t*) data.bind->buffer;
      std::memset(data.bind->buffer, 0, sizeof(int8_t));
      return value;
    }
    case MYSQL_TYPE_SHORT: {
      value = *(int16_t*) data.bind->buffer;
      std::memset(data.bind->buffer, 0, sizeof(int16_t));
      return value;
    }
    case MYSQL_TYPE_LONG: {
      value = *(int32_t*) data.bind->buffer;
      std::memset(data.bind->buffer, 0, sizeof(int32_t));
      return value;
    }
    case MYSQL_TYPE_LONGLONG: {
      value = *(int64_t*) data.bind->buffer;
      std::memset(data.bind->buffer, 0, sizeof(int64_t));
      return value;
    }
  }

  throw std::runtime_error("[oatpp::mariadb::mapping::Deserializer::deInt()]: Error. Unknown OID.");
}

oatpp::Void Deserializer::deserializeString(const Deserializer* _this, const InData& data, const Type* type)
{
  (void) _this;
  (void) type;

  if(data.isNull) {
    return oatpp::String();
  }

  auto ptr = (const char*) data.bind->buffer;
  auto size = *data.bind->length;  // Use the actual data length

  oatpp::String value(ptr, size);
  std::memset(data.bind->buffer, 0, data.bind->buffer_length);
  return value;
}

oatpp::Void Deserializer::deserializeFloat32(const Deserializer* _this, const InData& data, const Type* type) {

  (void) _this;
  (void) type;

  if(data.isNull) {
    return oatpp::Float32();
  }

  float value;

  switch(data.oid) {
    case MYSQL_TYPE_LONG:
    case MYSQL_TYPE_FLOAT: {
      value = *(float*) data.bind->buffer;
      std::memset(data.bind->buffer, 0, sizeof(float));
      return oatpp::Float32(value);
    }
  }

  throw std::runtime_error("[oatpp::mariadb::mapping::Deserializer::deserializeFloat32()]: Error. Unknown OID.");

}

oatpp::Void Deserializer::deserializeFloat64(const Deserializer* _this, const InData& data, const Type* type) {
  OATPP_LOGD("Deserializer", "Deserializing Float64 value");
  
  if(data.isNull) {
    OATPP_LOGD("Deserializer", "Float64 value is null");
    return oatpp::Float64();
  }
  
  double value = 0;
  
  switch(data.oid) {
    case MYSQL_TYPE_TINY: {
      value = static_cast<double>(*static_cast<int8_t*>(data.bind->buffer));
      break;
    }
    case MYSQL_TYPE_SHORT: {
      value = static_cast<double>(*static_cast<int16_t*>(data.bind->buffer));
      break;
    }
    case MYSQL_TYPE_LONG: {
      value = static_cast<double>(*static_cast<int32_t*>(data.bind->buffer));
      break;
    }
    case MYSQL_TYPE_LONGLONG: {
      value = static_cast<double>(*static_cast<int64_t*>(data.bind->buffer));
      break;
    }
    case MYSQL_TYPE_FLOAT: {
      value = static_cast<double>(*static_cast<float*>(data.bind->buffer));
      break;
    }
    case MYSQL_TYPE_DOUBLE: {
      value = *static_cast<double*>(data.bind->buffer);
      break;
    }
    default:
      OATPP_LOGE("Deserializer", "Unsupported buffer type for Float64: %d", data.oid);
      throw std::runtime_error("Unsupported buffer type for Float64: " + std::to_string(data.oid));
  }
  
  OATPP_LOGD("Deserializer", "Float64 value: %f", value);
  return oatpp::Float64(value);
}

template<class IntWrapper>
oatpp::Void Deserializer::deserializeInt(const Deserializer* _this, const InData& data, const Type* type) {
  (void) _this;
  (void) type;

  if(data.isNull) {
    OATPP_LOGD("Deserializer", "Int value is null");
    return IntWrapper();
  }

  if (std::is_same<IntWrapper, oatpp::Int64>::value || std::is_same<IntWrapper, oatpp::UInt64>::value) {
    switch(data.oid) {
      case MYSQL_TYPE_LONGLONG: {
        if (data.bind->is_unsigned) {
          uint64_t value = *static_cast<uint64_t*>(data.bind->buffer);
          OATPP_LOGD("Deserializer", "Unsigned Int64 value: %llu", value);
          if (std::is_same<IntWrapper, oatpp::UInt64>::value) {
            return IntWrapper(value);
          } else {
            return IntWrapper(static_cast<int64_t>(value));
          }
        } else {
          int64_t value = *static_cast<int64_t*>(data.bind->buffer);
          OATPP_LOGD("Deserializer", "Signed Int64 value: %lld", value);
          if (std::is_same<IntWrapper, oatpp::UInt64>::value) {
            return IntWrapper(static_cast<uint64_t>(value));
          } else {
            return IntWrapper(value);
          }
        }
      }
      default: {
        auto value = deInt(data);
        if (std::is_same<IntWrapper, oatpp::UInt64>::value) {
          return IntWrapper(static_cast<uint64_t>(value));
        } else {
          return IntWrapper(value);
        }
      }
    }
  }

  auto value = deInt(data);
  return IntWrapper((typename IntWrapper::UnderlyingType) value);
}

// Explicit template instantiations
template oatpp::Void Deserializer::deserializeInt<oatpp::Int8>(const Deserializer* _this, const InData& data, const Type* type);
template oatpp::Void Deserializer::deserializeInt<oatpp::UInt8>(const Deserializer* _this, const InData& data, const Type* type);
template oatpp::Void Deserializer::deserializeInt<oatpp::Int16>(const Deserializer* _this, const InData& data, const Type* type);
template oatpp::Void Deserializer::deserializeInt<oatpp::UInt16>(const Deserializer* _this, const InData& data, const Type* type);
template oatpp::Void Deserializer::deserializeInt<oatpp::Int32>(const Deserializer* _this, const InData& data, const Type* type);
template oatpp::Void Deserializer::deserializeInt<oatpp::UInt32>(const Deserializer* _this, const InData& data, const Type* type);
template oatpp::Void Deserializer::deserializeInt<oatpp::Int64>(const Deserializer* _this, const InData& data, const Type* type);
template oatpp::Void Deserializer::deserializeInt<oatpp::UInt64>(const Deserializer* _this, const InData& data, const Type* type);

oatpp::Void Deserializer::deserializeBoolean(const Deserializer* _this, const InData& data, const Type* type) {
  (void) _this;
  (void) type;

  if(data.isNull) {
    OATPP_LOGD("Deserializer", "Deserializing null boolean value");
    return oatpp::Boolean();
  }

  switch(data.oid) {
    case MYSQL_TYPE_BIT: {
      uint64_t value = *static_cast<uint64_t*>(data.bind->buffer);
      OATPP_LOGD("Deserializer", "Deserializing BIT value: %llu", value);
      return oatpp::Boolean(value != 0);
    }
    case MYSQL_TYPE_TINY: {
      signed char value = *static_cast<signed char*>(data.bind->buffer);
      OATPP_LOGD("Deserializer", "Deserializing boolean value: %d", (int)value);
      return oatpp::Boolean(value != 0);
    }
    default:
      OATPP_LOGD("Deserializer", "Unsupported buffer type: %d", data.oid);
      throw std::runtime_error("[oatpp::mariadb::mapping::Deserializer::deserializeBoolean()]: Error. Unsupported buffer type: " + std::to_string(data.oid));
  }
}

oatpp::Void Deserializer::deserializeAny(const Deserializer* _this, const InData& inData, const Type* type) {

  (void) type;

  if(inData.isNull) {
    return oatpp::Void(Any::Class::getType());
  }

  const Type* valueType;

  switch(inData.oid) {
    case MYSQL_TYPE_TINY:
      if (inData.bind->is_unsigned) {
        auto value = *(static_cast<unsigned char*>(inData.bind->buffer));
        if (type == oatpp::Boolean::Class::getType()) {
          return oatpp::Boolean(value != 0);
        }
        return oatpp::UInt8(value);
      } else {
        auto value = *(static_cast<signed char*>(inData.bind->buffer));
        if (type == oatpp::Boolean::Class::getType()) {
          return oatpp::Boolean(value != 0);
        }
        return oatpp::Int8(value);
      }
    case MYSQL_TYPE_SHORT:
      valueType = oatpp::Int16::Class::getType();
      break;
    case MYSQL_TYPE_LONG:
      valueType = oatpp::Int32::Class::getType();
      break;
    case MYSQL_TYPE_LONGLONG:
      valueType = oatpp::Int64::Class::getType();
      break;
    case MYSQL_TYPE_FLOAT:
      valueType = oatpp::Float32::Class::getType();
      break;
    case MYSQL_TYPE_DOUBLE:
      valueType = oatpp::Float64::Class::getType();
      break;
    case MYSQL_TYPE_STRING:
      valueType = oatpp::String::Class::getType();
      break;
    case MYSQL_TYPE_BIT:
      if (type == oatpp::UInt64::Class::getType()) {
        return oatpp::UInt64(*static_cast<v_uint64*>(inData.bind->buffer));
      }
      valueType = oatpp::UInt64::Class::getType();
      break;
    default:
      throw std::runtime_error("[oatpp::mariadb::mapping::Deserializer::deserializeAny()]: Error. Unknown OID.");
  }

  auto value = _this->deserialize(inData, valueType);
  auto anyHandle = std::make_shared<data::mapping::type::AnyHandle>(value.getPtr(), value.getValueType());
  return oatpp::Void(anyHandle, Any::Class::getType());

}

oatpp::Void Deserializer::deserializeEnum(const Deserializer* _this, const InData& data, const Type* type) {

  auto polymorphicDispatcher = static_cast<const data::mapping::type::__class::AbstractEnum::PolymorphicDispatcher*>(
    type->polymorphicDispatcher
  );

  data::mapping::type::EnumInterpreterError e = data::mapping::type::EnumInterpreterError::OK;
  const auto& value = _this->deserialize(data, polymorphicDispatcher->getInterpretationType());

  const auto& result = polymorphicDispatcher->fromInterpretation(value, e);

  if(e == data::mapping::type::EnumInterpreterError::OK) {
    return result;
  }

  switch(e) {
    case data::mapping::type::EnumInterpreterError::CONSTRAINT_NOT_NULL:
      throw std::runtime_error("[oatpp::mariadb::mapping::Deserializer::deserializeEnum()]: Error. Enum constraint violated - 'NotNull'.");

    default:
      throw std::runtime_error("[oatpp::mariadb::mapping::Deserializer::deserializeEnum()]: Error. Can't deserialize Enum.");
  }

}

}}}
