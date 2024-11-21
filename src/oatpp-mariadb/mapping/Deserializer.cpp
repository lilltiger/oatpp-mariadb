#include "Deserializer.hpp"

namespace oatpp { namespace mariadb { namespace mapping {

Deserializer::InData::InData(MYSQL_BIND* pBind,
                             const std::shared_ptr<const data::mapping::TypeResolver>& pTypeResolver)
{
  bind = pBind;
  typeResolver = pTypeResolver;
  oid = bind->buffer_type;
  isNull = (*bind->is_null == 1);
}

Deserializer::Deserializer() {

  m_methods.resize(data::mapping::type::ClassId::getClassCount(), nullptr);

  setDeserializerMethod(data::mapping::type::__class::String::CLASS_ID, &Deserializer::deserializeString);
  setDeserializerMethod(data::mapping::type::__class::Any::CLASS_ID, &Deserializer::deserializeAny);

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
  auto size = std::strlen(ptr);             // not including null-terminator
  // TODO: check buffer_length vs size
  size = std::min(size, data.bind->buffer_length - 1);

  oatpp::String value(ptr, size);

  // OATPP_LOGD("Deserializer::deserializeString()", "value='%s', size=%d, buffer_length=%d", value->c_str(), size, data.bind->buffer_length);

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

  (void) _this;
  (void) type;

  if(data.isNull) {
    return oatpp::Float64();
  }

  double value;

  switch(data.oid) {
    case MYSQL_TYPE_LONGLONG:
    case MYSQL_TYPE_DOUBLE: {
      value = *(double*) data.bind->buffer;
      std::memset(data.bind->buffer, 0, sizeof(double));
      return oatpp::Float64(value);
    }
  }

  throw std::runtime_error("[oatpp::mariadb::mapping::Deserializer::deserializeFloat64()]: Error. Unknown OID.");

}

oatpp::Void Deserializer::deserializeAny(const Deserializer* _this, const InData& data, const Type* type) {

  (void) type;

  if(data.isNull) {
    return oatpp::Void(Any::Class::getType());
  }

  const Type* valueType;

  switch(data.oid) {
    case MYSQL_TYPE_TINY:
      valueType = oatpp::Int8::Class::getType();
      break;
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
    default:
      throw std::runtime_error("[oatpp::mariadb::mapping::Deserializer::deserializeAny()]: Error. Unknown OID.");
  }

  auto value = _this->deserialize(data, valueType);
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
