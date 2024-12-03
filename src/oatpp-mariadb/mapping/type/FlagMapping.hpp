#ifndef oatpp_mariadb_mapping_type_FlagMapping_hpp
#define oatpp_mariadb_mapping_type_FlagMapping_hpp

#include "oatpp-mariadb/types/Flag.hpp"
#include "oatpp-mariadb/mapping/ResultMapper.hpp"

namespace oatpp { namespace mariadb { namespace mapping { namespace type {

template<v_uint32 N>
class FlagMapping {
public:
  static void install(ResultMapper& mapper) {
    mapper.setReadOneRowMethod(types::Flag<N>::CLASS_ID,
      [](ResultMapper* _this, ResultMapper::ResultData* dbData, const Type* type) -> oatpp::Void {
        if (!dbData->hasMore) {
          return oatpp::Void(nullptr);
        }

        auto& bind = dbData->bindResults[0];
        if (!*bind.is_null && bind.buffer && bind.buffer_length > 0) {
          const unsigned char* bytes = static_cast<const unsigned char*>(bind.buffer);
          v_uint64 val = 0;
          
          // Convert from little-endian byte order
          for(size_t i = 0; i < std::min(bind.buffer_length, size_t(8)); i++) {
            val |= static_cast<v_uint64>(bytes[i]) << (i * 8);
          }
          
          return oatpp::Void(std::make_shared<types::Flag<N>>(val));
        }
        
        return oatpp::Void(nullptr);
      });
  }
};

}}}}

#endif // oatpp_mariadb_mapping_type_FlagMapping_hpp 