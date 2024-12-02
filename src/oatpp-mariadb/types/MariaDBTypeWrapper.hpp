#ifndef oatpp_mariadb_types_MariaDBTypeWrapper_hpp
#define oatpp_mariadb_types_MariaDBTypeWrapper_hpp

#include "oatpp/core/Types.hpp"
#include <regex>
#include <algorithm>
#include <cctype>
#include <sstream>

namespace oatpp { namespace mariadb { namespace types {

// Validation context for flexible validation rules
struct ValidationContext {
    bool isStrict = false;
    bool allowNull = true;
    bool normalizeValues = true;
};

// Type wrapper base template
template<typename T, typename UnderlyingType>
class MariaDBTypeWrapper {
protected:
    UnderlyingType value;
    mutable UnderlyingType normalizedValue;
    mutable bool isNormalized = false;

public:
    explicit MariaDBTypeWrapper(const UnderlyingType& v) : value(v) {}
    
    // Basic accessors
    UnderlyingType getValue() const { return value; }
    bool isNull() const { return !value; }
    
    // Validation methods
    virtual bool validate() const = 0;
    virtual bool validate(const ValidationContext& context) const = 0;
    
    virtual bool validateStrict() const {
        return validate() && validateLength();
    }
    
    virtual bool validateLength() const {
        return !value || value->length() <= getMaxLength();
    }
    
    // Type information
    virtual oatpp::String getTypeName() const = 0;
    virtual std::size_t getMaxLength() const { return 255; }
    
    // Database mapping
    virtual oatpp::String getDbType() const {
        std::ostringstream oss;
        oss << "VARCHAR(" << getMaxLength() << ")";
        return oatpp::String(oss.str().c_str());
    }
    
    virtual oatpp::String getDbConstraints() const {
        return "";
    }
    
    // Value transformation
    virtual UnderlyingType normalize() const {
        if (!isNormalized) {
            normalizedValue = value;
            isNormalized = true;
        }
        return normalizedValue;
    }
    
    bool isDirty() const {
        return value != normalize();
    }
    
    // Error handling
    virtual oatpp::String getValidationError() const = 0;
    
    // Database value conversion
    virtual UnderlyingType toDbValue() const {
        return normalize();
    }
    
    static T fromDbValue(const UnderlyingType& dbValue) {
        return T(dbValue);
    }
    
    // Comparison operators
    bool operator==(const MariaDBTypeWrapper& other) const {
        if (isNull() && other.isNull()) return true;
        if (isNull() || other.isNull()) return false;
        return value == other.value;
    }
    
    bool operator!=(const MariaDBTypeWrapper& other) const {
        return !(*this == other);
    }
    
    bool operator<(const MariaDBTypeWrapper& other) const {
        if (isNull()) return !other.isNull();
        if (other.isNull()) return false;
        if constexpr(std::is_same_v<UnderlyingType, oatpp::String>) {
            return std::string(value->c_str()) < std::string(other.value->c_str());
        } else {
            return value < other.value;
        }
    }
};

// String-specific type wrapper
template<typename T>
class MariaDBStringTypeWrapper : public MariaDBTypeWrapper<T, oatpp::String> {
public:
    explicit MariaDBStringTypeWrapper(const oatpp::String& v) 
        : MariaDBTypeWrapper<T, oatpp::String>(v) {}
    
    bool validateLength() const override {
        return !this->value || this->value->length() <= this->getMaxLength();
    }
};

}}}

#endif // oatpp_mariadb_types_MariaDBTypeWrapper_hpp 