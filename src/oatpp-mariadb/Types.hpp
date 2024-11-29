#ifndef oatpp_mariadb_Types_hpp
#define oatpp_mariadb_Types_hpp

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/Types.hpp"

namespace oatpp { namespace mariadb { namespace types {

/**
 * SQL type mapping macros for explicit type declarations
 */

#define PARAM_VARCHAR(name, length) PARAM(oatpp::String, name)
#define PARAM_DECIMAL(name, precision, scale) PARAM(oatpp::Float64, name)
#define PARAM_INT(name) PARAM(oatpp::Int32, name)
#define PARAM_BIGINT(name) PARAM(oatpp::Int64, name)
#define PARAM_BOOL(name) PARAM(oatpp::Boolean, name)
#define PARAM_TEXT(name) PARAM(oatpp::String, name)
#define PARAM_DATETIME(name) PARAM(oatpp::String, name)
#define PARAM_DATE(name) PARAM(oatpp::String, name)
#define PARAM_TIME(name) PARAM(oatpp::String, name)
#define PARAM_BLOB(name) PARAM(oatpp::String, name)

}}}

#endif // oatpp_mariadb_Types_hpp 