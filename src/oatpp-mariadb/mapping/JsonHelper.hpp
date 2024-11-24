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
    config->includeNullFields = true;
    config->alwaysIncludeNullCollectionElements = true;

    // Register Int64 serializer
    mapper.getSerializer()->setSerializerMethod(data::mapping::type::__class::Int64::CLASS_ID, 
      [](oatpp::parser::json::mapping::Serializer* serializer,
         oatpp::data::stream::ConsistentOutputStream* stream,
         const oatpp::Void& polymorph) {
        if (!polymorph) {
          stream->writeSimple("null", 4);
          return;
        }
        auto value = polymorph.cast<oatpp::Int64>();
        stream->writeAsString((v_int64)value);
      });

    // Register UInt64 serializer
    mapper.getSerializer()->setSerializerMethod(data::mapping::type::__class::UInt64::CLASS_ID,
      [](oatpp::parser::json::mapping::Serializer* serializer,
         oatpp::data::stream::ConsistentOutputStream* stream,
         const oatpp::Void& polymorph) {
        if (!polymorph) {
          stream->writeSimple("null", 4);
          return;
        }
        auto value = polymorph.cast<oatpp::UInt64>();
        stream->writeAsString((v_uint64)value);
      });

    // Register object serializer (needed for proper Void type handling)
    mapper.getSerializer()->setSerializerMethod(data::mapping::type::__class::AbstractObject::CLASS_ID,
      [](oatpp::parser::json::mapping::Serializer* serializer,
         oatpp::data::stream::ConsistentOutputStream* stream,
         const oatpp::Void& polymorph) {
        if (!polymorph) {
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
  }
};

}}}

#endif // oatpp_mariadb_mapping_JsonHelper_hpp
