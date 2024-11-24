#ifndef oatpp_mariadb_mapping_JsonHelper_hpp
#define oatpp_mariadb_mapping_JsonHelper_hpp

#include "oatpp/core/Types.hpp"
#include "oatpp/core/data/mapping/type/Type.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/parser/json/mapping/Serializer.hpp"
#include "oatpp/parser/json/mapping/Deserializer.hpp"

namespace oatpp { namespace mariadb { namespace mapping {

/**
 * Helper class for JSON serialization of Int64 and UInt64 types.
 */
class JsonHelper {
public:
  static void setupIntegerSerializers(oatpp::parser::json::mapping::ObjectMapper& mapper) {
    auto config = mapper.getSerializer()->getConfig();
    config->enabledInterpretations = {"Int64", "UInt64"};
    config->includeNullFields = true;  // Ensure null fields are included
    config->alwaysIncludeNullCollectionElements = true;  // Important for Vector serialization

    OATPP_LOGD("JsonHelper", "Setting up integer serializers with includeNullFields=%d, alwaysIncludeNullCollectionElements=%d", 
               config->includeNullFields, config->alwaysIncludeNullCollectionElements);

    // Register custom serializers for Int64 and UInt64
    mapper.getSerializer()->setSerializerMethod(data::mapping::type::__class::Int64::CLASS_ID, 
      [](oatpp::parser::json::mapping::Serializer* serializer,
         oatpp::data::stream::ConsistentOutputStream* stream,
         const oatpp::Void& polymorph) {
        OATPP_LOGD("JsonHelper", "Int64 serializer called. ValueType=%s, HasValue=%d", 
                   (polymorph.getValueType() ? polymorph.getValueType()->classId.name : "null"),
                   (bool)polymorph);
        if (polymorph.getValueType() == nullptr || !polymorph) {
          OATPP_LOGD("JsonHelper", "Writing null for Int64");
          stream->writeSimple("null", 4);
          return;
        }
        auto value = polymorph.cast<oatpp::Int64>();
        OATPP_LOGD("JsonHelper", "Writing Int64 value: %lld", (v_int64)value);
        stream->writeAsString((v_int64)value);
      });

    mapper.getSerializer()->setSerializerMethod(data::mapping::type::__class::UInt64::CLASS_ID,
      [](oatpp::parser::json::mapping::Serializer* serializer,
         oatpp::data::stream::ConsistentOutputStream* stream,
         const oatpp::Void& polymorph) {
        OATPP_LOGD("JsonHelper", "UInt64 serializer called. ValueType=%s, HasValue=%d", 
                   (polymorph.getValueType() ? polymorph.getValueType()->classId.name : "null"),
                   (bool)polymorph);
        if (polymorph.getValueType() == nullptr || !polymorph) {
          OATPP_LOGD("JsonHelper", "Writing null for UInt64");
          stream->writeSimple("null", 4);
          return;
        }
        auto value = polymorph.cast<oatpp::UInt64>();
        OATPP_LOGD("JsonHelper", "Writing UInt64 value: %llu", (v_uint64)value);
        stream->writeAsString((v_uint64)value);
      });

    // Register object serializer
    mapper.getSerializer()->setSerializerMethod(data::mapping::type::__class::AbstractObject::CLASS_ID,
      [](oatpp::parser::json::mapping::Serializer* serializer,
         oatpp::data::stream::ConsistentOutputStream* stream,
         const oatpp::Void& polymorph) {
        OATPP_LOGD("JsonHelper", "Object serializer called. ValueType=%s, HasValue=%d", 
                   (polymorph.getValueType() ? polymorph.getValueType()->classId.name : "null"),
                   (bool)polymorph);

        if (polymorph.getValueType() == nullptr || !polymorph) {
          OATPP_LOGD("JsonHelper", "Writing null for Object");
          stream->writeSimple("null", 4);
          return;
        }

        auto* object = static_cast<oatpp::data::mapping::type::BaseObject*>(polymorph.get());
        auto dispatcher = static_cast<const oatpp::data::mapping::type::__class::AbstractObject::PolymorphicDispatcher*>(
          polymorph.getValueType()->polymorphicDispatcher
        );
        auto properties = dispatcher->getProperties();

        stream->writeSimple("{", 1);
        bool first = true;

        for(const auto& p : properties->getList()) {
          auto value = p->get(object);
          
          if (!first) {
            stream->writeSimple(",", 1);
          }

          // Write the property name as a JSON string
          stream->writeSimple("\"", 1);
          stream->writeSimple(p->name, std::strlen(p->name));
          stream->writeSimple("\":", 2);
          
          if (value) {
            serializer->serializeToStream(stream, value);
          } else {
            stream->writeSimple("null", 4);
          }
          
          first = false;
        }
        
        stream->writeSimple("}", 1);
      });

    // Register collection serializer
    mapper.getSerializer()->setSerializerMethod(data::mapping::type::__class::AbstractVector::CLASS_ID,
      [](oatpp::parser::json::mapping::Serializer* serializer,
         oatpp::data::stream::ConsistentOutputStream* stream,
         const oatpp::Void& polymorph) {
        OATPP_LOGD("JsonHelper", "Vector serializer called. ValueType=%s, HasValue=%d", 
                   (polymorph.getValueType() ? polymorph.getValueType()->classId.name : "null"),
                   (bool)polymorph);

        if (polymorph.getValueType() == nullptr || !polymorph) {
          OATPP_LOGD("JsonHelper", "Writing null for Vector");
          stream->writeSimple("null", 4);
          return;
        }

        const auto* type = polymorph.getValueType();
        if (!type->isCollection) {
          OATPP_LOGD("JsonHelper", "Invalid type - not a collection");
          throw std::runtime_error("Invalid type - not a collection");
        }

        auto dispatcher = static_cast<const oatpp::data::mapping::type::__class::Collection::PolymorphicDispatcher*>(type->polymorphicDispatcher);
        auto iterator = dispatcher->beginIteration(polymorph);
        
        stream->writeSimple("[", 1);
        bool first = true;
        
        while(!iterator->finished()) {
          auto item = iterator->get();
          if (!first) {
            stream->writeSimple(",", 1);
          }
          
          if (item) {
            serializer->serializeToStream(stream, item);
          } else {
            stream->writeSimple("null", 4);
          }
          
          iterator->next();
          first = false;
        }
        
        stream->writeSimple("]", 1);
      });
    
    OATPP_LOGD("JsonHelper", "Integer, object, and collection serializers setup complete");
  }
};

}}}

#endif // oatpp_mariadb_mapping_JsonHelper_hpp
