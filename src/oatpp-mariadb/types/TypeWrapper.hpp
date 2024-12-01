#ifndef oatpp_mariadb_types_TypeWrapper_hpp
#define oatpp_mariadb_types_TypeWrapper_hpp

#include "oatpp/core/Types.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"

namespace oatpp { namespace mariadb { namespace types {

/**
 * Base template for creating custom type wrappers around MariaDB types.
 * Provides validation, serialization, and database constraint support.
 */
template<typename T, typename UnderlyingType>
class TypeWrapper {
protected:
    UnderlyingType value;
public:
    /**
     * Constructor
     * @param v - The underlying value
     */
    explicit TypeWrapper(const UnderlyingType& v) : value(v) {}
    
    /**
     * Get the underlying value
     * @return The wrapped value
     */
    UnderlyingType getValue() const { return value; }
    
    /**
     * Validate the value according to type-specific rules
     * @return true if valid, false otherwise
     */
    virtual bool validate() const = 0;
    
    /**
     * Get the SQL constraints for this type
     * @return SQL constraint definition
     */
    virtual oatpp::String getDbConstraints() const = 0;
    
    /**
     * Serialize the value to string
     * @return Serialized value
     */
    virtual oatpp::String serialize() const {
        oatpp::parser::json::mapping::ObjectMapper mapper;
        return mapper.writeToString(value);
    }
    
    /**
     * Deserialize from string
     * @param str - The string to deserialize
     * @return Deserialized object
     */
    static T deserialize(const oatpp::String& str) {
        oatpp::parser::json::mapping::ObjectMapper mapper;
        auto value = mapper.readFromString<UnderlyingType>(str);
        return T(value);
    }
    
    /**
     * Get the SQL type for this wrapper
     * @return SQL type definition
     */
    virtual oatpp::String getSqlType() const = 0;
    
    /**
     * Check if the value can be null
     * @return true if nullable, false otherwise
     */
    virtual bool isNullable() const { return false; }
    
    /**
     * Get additional SQL constraints (e.g., UNIQUE)
     * @return Additional SQL constraints
     */
    virtual oatpp::String getAdditionalConstraints() const { return ""; }
};

}}}

#endif // oatpp_mariadb_types_TypeWrapper_hpp 