#ifndef oatpp_mariadb_types_Flag_hpp
#define oatpp_mariadb_types_Flag_hpp

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp-mariadb/mapping/Serializer.hpp"
#include <unordered_map>
#include <string>
#include <mysql/mysql.h>

namespace oatpp { namespace mariadb { namespace types {

template<v_uint32 N>
class Flag;

#include OATPP_CODEGEN_BEGIN(DTO)

template<v_uint32 N>
class Flag : public oatpp::UInt64 {
  DTO_INIT(Flag, UInt64)

private:
  static std::unordered_map<std::string, v_uint64> s_flagValues;
  static std::unordered_map<std::string, std::vector<std::string>> s_flagInheritance;

public:
  typedef Flag<N> __Flag;
  static const oatpp::data::mapping::type::ClassId CLASS_ID;

  Flag() : UInt64((v_uint64)0) {}
  Flag(v_uint64 val) : UInt64(val) {}
  Flag(const std::shared_ptr<typename UInt64::ObjectType>& ptr, const oatpp::data::mapping::type::Type* const valueType)
    : UInt64(ptr, valueType) {}

  void setFlag(const std::string& name) {
    auto it = s_flagValues.find(name);
    if (it != s_flagValues.end()) {
      *this = Flag<N>(this->getValue((v_uint64)0) | it->second);
    }
  }

  void setFlagWithInheritance(const std::string& name) {
    setFlag(name);
    auto it = s_flagInheritance.find(name);
    if (it != s_flagInheritance.end()) {
      for (const auto& child : it->second) {
        setFlag(child);
      }
    }
  }

  bool hasFlag(const std::string& name) const {
    auto it = s_flagValues.find(name);
    if (it != s_flagValues.end()) {
      return (this->getValue((v_uint64)0) & it->second) == it->second;
    }
    return false;
  }

  void clearFlag(const std::string& name) {
    auto it = s_flagValues.find(name);
    if (it != s_flagValues.end()) {
      *this = Flag<N>(this->getValue((v_uint64)0) & ~it->second);
    }
  }

  void clearAllFlags() {
    *this = Flag<N>((v_uint64)0);
  }

  static void registerFlag(const std::string& name, v_uint64 val) {
    s_flagValues[name] = val;
  }

  static void registerFlagInheritance(const std::string& parent, const std::string& child) {
    s_flagInheritance[parent].push_back(child);
  }

  static std::shared_ptr<Flag<N>> createShared() {
    return std::make_shared<Flag<N>>();
  }

  static std::shared_ptr<Flag<N>> createShared(v_uint64 val) {
    return std::make_shared<Flag<N>>(val);
  }

  // Get the SQL type for this flag
  oatpp::String getDbType() const {
    return oatpp::String(std::string("BIT(") + std::to_string(N) + ")");
  }

  // Add serialization support
  static void setupSerializer(oatpp::mariadb::mapping::Serializer& serializer) {
    serializer.setSerializerMethod(Flag<N>::CLASS_ID,
      [](const oatpp::mariadb::mapping::Serializer* _this,
         MYSQL_STMT* stmt,
         v_uint32 paramIndex,
         const oatpp::Void& polymorph) -> void {
        if(paramIndex >= _this->getBindParams().size()) {
          _this->getBindParams().resize(paramIndex + 1);
        }
        
        auto& bind = _this->getBindParams()[paramIndex];
        std::memset(&bind, 0, sizeof(MYSQL_BIND));
        
        if(polymorph) {
          auto value = static_cast<Flag<N>*>(polymorph.get());
          if(value) {
            bind.buffer_type = MYSQL_TYPE_BIT;
            bind.buffer = malloc(8);  // Always use 8 bytes for BIT(64)
            if(!bind.buffer) {
              throw std::runtime_error("Failed to allocate memory for Flag value");
            }
            
            // Get the value and handle endianness
            v_uint64 val = value->getValue((v_uint64)0);
            unsigned char* bytes = static_cast<unsigned char*>(bind.buffer);
            
            // MariaDB expects the bytes in little-endian order
            for(size_t i = 0; i < 8; i++) {
              bytes[i] = (val >> (i * 8)) & 0xFF;
            }
            
            bind.buffer_length = 8;
            bind.is_null_value = 0;
            bind.length_value = 8;
          } else {
            bind.buffer_type = MYSQL_TYPE_NULL;
            bind.is_null_value = 1;
          }
        } else {
          bind.buffer_type = MYSQL_TYPE_NULL;
          bind.is_null_value = 1;
        }
        
        bind.is_null = &bind.is_null_value;
        bind.length = &bind.length_value;
      });
  }
};

template<v_uint32 N>
std::unordered_map<std::string, v_uint64> Flag<N>::s_flagValues;

template<v_uint32 N>
std::unordered_map<std::string, std::vector<std::string>> Flag<N>::s_flagInheritance;

template<v_uint32 N>
const oatpp::data::mapping::type::ClassId Flag<N>::CLASS_ID("Flag");

#include OATPP_CODEGEN_END(DTO)

}}}

#endif // oatpp_mariadb_types_Flag_hpp