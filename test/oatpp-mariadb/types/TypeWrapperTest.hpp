#ifndef oatpp_test_mariadb_types_TypeWrapperTest_hpp
#define oatpp_test_mariadb_types_TypeWrapperTest_hpp

#include "oatpp-test/UnitTest.hpp"
#include "oatpp-mariadb/orm.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/Types.hpp"
#include <regex>
#include <algorithm>
#include <cctype>
#include <sstream>

namespace oatpp { namespace test { namespace mariadb { namespace types {

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
    
    // Serialization
    virtual UnderlyingType toDbValue() const {
        return normalize();
    }
    
    static T fromDbValue(const UnderlyingType& dbValue) {
        return T(dbValue);
    }
    
    // Operators
    bool operator==(const MariaDBTypeWrapper& other) const {
        if (isNull() && other.isNull()) return true;
        if (isNull() || other.isNull()) return false;
        return std::string(value->c_str()) == std::string(other.value->c_str());
    }
    
    bool operator!=(const MariaDBTypeWrapper& other) const {
        return !(*this == other);
    }
    
    bool operator<(const MariaDBTypeWrapper& other) const {
        if (isNull()) return !other.isNull();
        if (other.isNull()) return false;
        return std::string(value->c_str()) < std::string(other.value->c_str());
    }
};

// Email type implementation
class Email : public MariaDBTypeWrapper<Email, oatpp::String> {
private:
    static const std::regex& getPattern() {
        static const std::regex pattern(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
        return pattern;
    }

public:
    explicit Email(const oatpp::String& email) 
        : MariaDBTypeWrapper<Email, oatpp::String>(email) {}
    
    bool validate() const override {
        if (!value) return true;  // Null is valid unless specified in context
        return std::regex_match(value->c_str(), getPattern());
    }
    
    bool validate(const ValidationContext& context) const override {
        if (!value && !context.allowNull) return false;
        if (context.normalizeValues) {
            normalize();
        }
        return context.isStrict ? validateStrict() : validate();
    }
    
    oatpp::String normalize() const override {
        if (!isNormalized) {
            if (value) {
                std::string lowered = value->c_str();
                std::transform(lowered.begin(), lowered.end(), lowered.begin(), ::tolower);
                normalizedValue = oatpp::String(lowered.c_str());
            } else {
                normalizedValue = nullptr;
            }
            isNormalized = true;
        }
        return normalizedValue;
    }
    
    oatpp::String getTypeName() const override {
        return oatpp::String("Email");
    }
    
    std::size_t getMaxLength() const override {
        return 255;
    }
    
    oatpp::String getDbConstraints() const override {
        return "CHECK (email REGEXP '^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$')";
    }
    
    oatpp::String getValidationError() const override {
        std::ostringstream oss;
        if (!validate()) {
            if (isNull()) {
                oss << getTypeName()->c_str() << " cannot be null";
            } else if (!std::regex_match(value->c_str(), getPattern())) {
                oss << getTypeName()->c_str() << " format is invalid";
            }
        } else if (isDirty()) {
            oss << getTypeName()->c_str() << " requires normalization";
        } else if (!validateLength()) {
            oss << getTypeName()->c_str() << " exceeds maximum length of " << getMaxLength();
        }
        return oatpp::String(oss.str().c_str());
    }
};

// Phone number type implementation
class PhoneNumber : public MariaDBTypeWrapper<PhoneNumber, oatpp::String> {
private:
    static const std::regex& getPattern() {
        static const std::regex pattern(R"(\+\d{1,3}-\d{3}-\d{3}-\d{4})");
        return pattern;
    }

public:
    explicit PhoneNumber(const oatpp::String& phone) 
        : MariaDBTypeWrapper<PhoneNumber, oatpp::String>(phone) {}
    
    bool validate() const override {
        if (!value) return true;
        auto normalizedVal = normalize();
        return std::regex_match(normalizedVal->c_str(), getPattern());
    }
    
    bool validate(const ValidationContext& context) const override {
        if (!value && !context.allowNull) return false;
        if (context.normalizeValues) {
            normalize();
            return context.isStrict ? validateStrict() : validate();
        }
        return context.isStrict ? validateStrict() : std::regex_match(value->c_str(), getPattern());
    }
    
    oatpp::String normalize() const override {
        if (!isNormalized) {
            if (value) {
                std::string temp = value->c_str();
                temp.erase(std::remove(temp.begin(), temp.end(), ' '), temp.end());
                normalizedValue = oatpp::String(temp.c_str());
            } else {
                normalizedValue = nullptr;
            }
            isNormalized = true;
        }
        return normalizedValue;
    }
    
    oatpp::String getTypeName() const override {
        return oatpp::String("Phone Number");
    }
    
    std::size_t getMaxLength() const override {
        return 20;
    }
    
    oatpp::String getDbConstraints() const override {
        return "CHECK (phone REGEXP '^\\\\+[0-9]{1,3}-[0-9]{3}-[0-9]{3}-[0-9]{4}$')";
    }
    
    oatpp::String getValidationError() const override {
        std::ostringstream oss;
        if (!validate()) {
            if (isNull()) {
                oss << getTypeName()->c_str() << " cannot be null";
            } else if (!std::regex_match(value->c_str(), getPattern())) {
                oss << getTypeName()->c_str() << " format is invalid";
            }
        } else if (isDirty()) {
            oss << getTypeName()->c_str() << " requires normalization";
        } else if (!validateLength()) {
            oss << getTypeName()->c_str() << " exceeds maximum length of " << getMaxLength();
        }
        return oatpp::String(oss.str().c_str());
    }
};

#include OATPP_CODEGEN_BEGIN(DTO)

class TypeWrapperRow : public oatpp::DTO {
    DTO_INIT(TypeWrapperRow, DTO)
    
    DTO_FIELD(String, email);
    DTO_FIELD(String, phone);
    DTO_FIELD(String, name);
};

#include OATPP_CODEGEN_END(DTO)

#include OATPP_CODEGEN_BEGIN(DbClient)

class TestClient : public oatpp::orm::DbClient {
public:
    explicit TestClient(const std::shared_ptr<oatpp::orm::Executor>& executor)
        : oatpp::orm::DbClient(executor)
    {}

    QUERY(createTable,
          "CREATE TABLE IF NOT EXISTS type_wrapper_test ("
          "  id INTEGER PRIMARY KEY AUTO_INCREMENT,"
          "  email VARCHAR(255) NOT NULL CHECK (email REGEXP '^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$'),"
          "  phone VARCHAR(20) NOT NULL CHECK (phone REGEXP '^\\\\+[0-9]{1,3}-[0-9]{3}-[0-9]{3}-[0-9]{4}$'),"
          "  name VARCHAR(100)"
          ") ENGINE=InnoDB;")

    QUERY(dropTable,
          "DROP TABLE IF EXISTS type_wrapper_test;")

    QUERY(insertRow,
          "INSERT INTO type_wrapper_test (email, phone, name) "
          "VALUES (:row.email, :row.phone, :row.name);",
          PARAM(oatpp::Object<TypeWrapperRow>, row))

    QUERY(selectAll,
          "SELECT * FROM type_wrapper_test ORDER BY id;")

    QUERY(deleteAll,
          "DELETE FROM type_wrapper_test;")
};

#include OATPP_CODEGEN_END(DbClient)

class TypeWrapperTest : public UnitTest {
public:
    TypeWrapperTest() : UnitTest("TEST[mariadb::types::TypeWrapperTest]") {}
    void onRun() override;
};

}}}}

#endif // oatpp_test_mariadb_types_TypeWrapperTest_hpp 