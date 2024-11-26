#ifndef oatpp_mariadb_dto_CountResult_hpp
#define oatpp_mariadb_dto_CountResult_hpp

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/Types.hpp"

namespace oatpp { namespace mariadb { namespace dto {

#include OATPP_CODEGEN_BEGIN(DTO)

class CountResult : public oatpp::DTO {
  DTO_INIT(CountResult, DTO);
  DTO_FIELD(Int64, count, "count");
};

#include OATPP_CODEGEN_END(DTO)

}}}

#endif // oatpp_mariadb_dto_CountResult_hpp
