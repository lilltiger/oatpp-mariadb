#ifndef oatpp_mariadb_types_CommonTypes_hpp
#define oatpp_mariadb_types_CommonTypes_hpp

#include "TypeWrapper.hpp"
#include <regex>

namespace oatpp { namespace mariadb { namespace types {

/**
 * Email type with validation
 */
class Email : public TypeWrapper<Email, oatpp::String> {
public:
    explicit Email(const oatpp::String& email) 
        : TypeWrapper<Email, oatpp::String>(email) {}
    
    bool validate() const override {
        if (!value) return false;
        static const std::regex pattern(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
        return std::regex_match(value->std_str(), pattern);
    }
    
    oatpp::String getDbConstraints() const override {
        return "CHECK (email REGEXP '^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$')";
    }
    
    oatpp::String getSqlType() const override {
        return "VARCHAR(255)";
    }
    
    oatpp::String getAdditionalConstraints() const override {
        return "UNIQUE";  // Emails should be unique
    }
};

/**
 * Phone number type with validation
 */
class PhoneNumber : public TypeWrapper<PhoneNumber, oatpp::String> {
public:
    explicit PhoneNumber(const oatpp::String& phone) 
        : TypeWrapper<PhoneNumber, oatpp::String>(phone) {}
    
    bool validate() const override {
        if (!value) return false;
        static const std::regex pattern(R"(\+\d{1,3}-\d{3}-\d{3}-\d{4})");
        return std::regex_match(value->std_str(), pattern);
    }
    
    oatpp::String getDbConstraints() const override {
        return "CHECK (phone REGEXP '^\\\+[0-9]{1,3}-[0-9]{3}-[0-9]{3}-[0-9]{4}$')";
    }
    
    oatpp::String getSqlType() const override {
        return "VARCHAR(20)";
    }
};

/**
 * URL type with validation
 */
class URL : public TypeWrapper<URL, oatpp::String> {
public:
    explicit URL(const oatpp::String& url) 
        : TypeWrapper<URL, oatpp::String>(url) {}
    
    bool validate() const override {
        if (!value) return false;
        static const std::regex pattern(
            R"(https?:\/\/(www\.)?[-a-zA-Z0-9@:%._\+~#=]{1,256}\.[a-zA-Z0-9()]{1,6}\b([-a-zA-Z0-9()@:%_\+.~#?&//=]*))"
        );
        return std::regex_match(value->std_str(), pattern);
    }
    
    oatpp::String getDbConstraints() const override {
        return "CHECK (url REGEXP '^https?:\\/\\/(www\\.)?[-a-zA-Z0-9@:%._\\+~#=]{1,256}\\.[a-zA-Z0-9()]{1,6}\\b([-a-zA-Z0-9()@:%_\\+.~#?&//=]*)$')";
    }
    
    oatpp::String getSqlType() const override {
        return "VARCHAR(2048)";
    }
};

/**
 * Currency amount with validation
 */
class CurrencyAmount : public TypeWrapper<CurrencyAmount, oatpp::Float64> {
public:
    explicit CurrencyAmount(const oatpp::Float64& amount) 
        : TypeWrapper<CurrencyAmount, oatpp::Float64>(amount) {}
    
    bool validate() const override {
        if (!value) return false;
        // Check for valid currency amount (non-negative, max 2 decimal places)
        double amount = *value;
        return amount >= 0 && 
               std::abs(amount - std::round(amount * 100) / 100) < 0.00001;
    }
    
    oatpp::String getDbConstraints() const override {
        return "CHECK (amount >= 0)";
    }
    
    oatpp::String getSqlType() const override {
        return "DECIMAL(10,2)";
    }
};

}}}

#endif // oatpp_mariadb_types_CommonTypes_hpp 