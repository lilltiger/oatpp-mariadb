#ifndef oatpp_test_mariadb_types_TypeWrapperTest_hpp
#define oatpp_test_mariadb_types_TypeWrapperTest_hpp

#include "oatpp-test/UnitTest.hpp"
#include "oatpp-mariadb/orm.hpp"
#include "oatpp-mariadb/types/MariaDBTypeWrapper.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/Types.hpp"
#include <regex>
#include <algorithm>
#include <cctype>
#include <sstream>

namespace oatpp { namespace test { namespace mariadb { namespace types {

#include OATPP_CODEGEN_BEGIN(DTO)

class TypeWrapperRow : public oatpp::DTO {
    DTO_INIT(TypeWrapperRow, DTO)
    
    DTO_FIELD(String, email);
    DTO_FIELD(String, phone);
    DTO_FIELD(String, name);
};

#include OATPP_CODEGEN_END(DTO)

// Email type with validation
class Email : public oatpp::mariadb::types::MariaDBTypeWrapper<Email, oatpp::String> {
public:
    explicit Email(const oatpp::String& email) 
        : MariaDBTypeWrapper<Email, oatpp::String>(email) {}
    
    bool validate() const override {
        if (!value) return false;
        static const std::regex pattern(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
        return std::regex_match(value->c_str(), pattern);
    }
    
    bool validate(const oatpp::mariadb::types::ValidationContext& context) const override {
        if (isNull()) return context.allowNull;
        if (context.normalizeValues) {
            normalize();
            return validate() && (!context.isStrict || validateLength());
        }
        if (!validate()) return false;
        if (context.isStrict && !validateLength()) return false;
        return !isDirty();
    }
    
    oatpp::String getTypeName() const override {
        return "Email";
    }
    
    oatpp::String getValidationError() const override {
        if (isNull()) return "Email cannot be null";
        if (!validate()) return "Invalid email format";
        if (!validateLength()) return "Email exceeds maximum length";
        return "";
    }
    
    oatpp::String getDbConstraints() const override {
        return "CHECK (email REGEXP '^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$')";
    }
    
    oatpp::String normalize() const override {
        if (!isNormalized) {
            if (!value) {
                normalizedValue = value;
            } else {
                std::string normalized = value->c_str();
                std::transform(normalized.begin(), normalized.end(), normalized.begin(), ::tolower);
                normalizedValue = normalized.c_str();
            }
            isNormalized = true;
        }
        return normalizedValue;
    }
};

// Phone number type with validation
class PhoneNumber : public oatpp::mariadb::types::MariaDBTypeWrapper<PhoneNumber, oatpp::String> {
public:
    explicit PhoneNumber(const oatpp::String& phone) 
        : MariaDBTypeWrapper<PhoneNumber, oatpp::String>(phone) {}
    
    bool validate() const override {
        if (!value) return false;
        static const std::regex pattern(R"(\+\d{1,3}-\d{3}-\d{3}-\d{4})");
        if (isNormalized && normalizedValue) {
            return std::regex_match(normalizedValue->c_str(), pattern);
        }
        return std::regex_match(value->c_str(), pattern);
    }
    
    bool validate(const oatpp::mariadb::types::ValidationContext& context) const override {
        if (isNull()) return context.allowNull;
        if (context.normalizeValues) {
            normalize();
            return validate() && (!context.isStrict || validateLength());
        }
        if (!validate()) return false;
        if (context.isStrict && !validateLength()) return false;
        return !isDirty();
    }
    
    oatpp::String getTypeName() const override {
        return "Phone Number";
    }
    
    std::size_t getMaxLength() const override {
        return 20;
    }
    
    oatpp::String getValidationError() const override {
        if (isNull()) return "Phone number cannot be null";
        if (!validate()) return "Invalid phone number format (should be +X-XXX-XXX-XXXX)";
        if (!validateLength()) return "Phone number exceeds maximum length";
        return "";
    }
    
    oatpp::String getDbConstraints() const override {
        return "CHECK (phone REGEXP '^\\\\+[0-9]{1,3}-[0-9]{3}-[0-9]{3}-[0-9]{4}$')";
    }
    
    oatpp::String normalize() const override {
        if (!isNormalized) {
            if (!value) {
                normalizedValue = value;
            } else {
                std::string normalized = value->c_str();
                // Remove all whitespace
                normalized.erase(std::remove_if(normalized.begin(), normalized.end(), ::isspace), normalized.end());
                normalizedValue = normalized.c_str();
            }
            isNormalized = true;
        }
        return normalizedValue;
    }
};

#include OATPP_CODEGEN_BEGIN(DbClient)

class TestClient : public oatpp::orm::DbClient {
public:
    explicit TestClient(const std::shared_ptr<oatpp::orm::Executor>& executor)
        : oatpp::orm::DbClient(executor)
    {}

    QUERY(createTable,
          "CREATE TABLE IF NOT EXISTS type_wrapper_test ("
          "  id INTEGER PRIMARY KEY AUTO_INCREMENT,"
          "  email VARCHAR(255) NOT NULL CHECK (email REGEXP '^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$'),"
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