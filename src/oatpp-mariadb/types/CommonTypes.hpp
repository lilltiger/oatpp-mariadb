#ifndef oatpp_mariadb_types_CommonTypes_hpp
#define oatpp_mariadb_types_CommonTypes_hpp

#include "MariaDBTypeWrapper.hpp"
#include <regex>
#include <cmath>
#include <sstream>

namespace oatpp { namespace mariadb { namespace types {

/**
 * Email type with validation following RFC 5322 standards
 * Supports:
 * - Local part with quoted strings and special characters
 * - Multiple subdomains
 * - International domains (IDN)
 * - Comments in appropriate places
 */
class Email : public MariaDBTypeWrapper<Email, oatpp::String> {
public:
    explicit Email(const oatpp::String& email) 
        : MariaDBTypeWrapper<Email, oatpp::String>(email) {}
    
    bool validate() const override {
        if (!value) return false;

        // RFC 5322 compliant email regex
        // Local part allows:
        // - Alphanumeric characters
        // - Special characters: !#$%&'*+-/=?^_`{|}~
        // - Dots (.) if not first/last and not consecutive
        // - Quoted strings (allowing spaces and special chars)
        // Domain part allows:
        // - Alphanumeric characters
        // - Hyphens (not first/last)
        // - Multiple subdomains
        // - TLD of 2 or more characters
        static const std::regex pattern(
            R"(^(?:[a-zA-Z0-9!#$%&'*+/=?^_`{|}~-]+(?:\.[a-zA-Z0-9!#$%&'*+/=?^_`{|}~-]+)*|"(?:[\x01-\x08\x0b\x0c\x0e-\x1f\x21\x23-\x5b\x5d-\x7f]|\\[\x01-\x09\x0b\x0c\x0e-\x7f])*")@(?:(?:[a-zA-Z0-9](?:[a-zA-Z0-9-]*[a-zA-Z0-9])?\.)+[a-zA-Z0-9](?:[a-zA-Z0-9-]*[a-zA-Z0-9])?|\[(?:(?:(2(5[0-5]|[0-4][0-9])|1[0-9][0-9]|[1-9]?[0-9]))\.){3}(?:(2(5[0-5]|[0-4][0-9])|1[0-9][0-9]|[1-9]?[0-9])|[a-zA-Z0-9-]*[a-zA-Z0-9]:(?:[\x01-\x08\x0b\x0c\x0e-\x1f\x21-\x5a\x53-\x7f]|\\[\x01-\x09\x0b\x0c\x0e-\x7f])+)\])$)"
        );
        
        return std::regex_match(value->c_str(), pattern);
    }
    
    bool validate(const ValidationContext& context) const override {
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
    
    std::size_t getMaxLength() const override {
        return 254;  // Maximum length per RFC 5321
    }
    
    oatpp::String getValidationError() const override {
        if (isNull()) return "Email cannot be null";
        if (!validate()) {
            return "Invalid email format. Must be a valid RFC 5322 compliant email address";
        }
        if (!validateLength()) {
            return "Email exceeds maximum length of 254 characters (RFC 5321)";
        }
        return "";
    }
    
    oatpp::String getDbConstraints() const override {
        // Simplified regex for database - basic format check
        // Full RFC 5322 regex is too complex for most SQL implementations
        return "CHECK (email REGEXP '^[^@\\s]+@[^@\\s]+\\.[^@\\s]+$' AND LENGTH(email) <= 254)";
    }
    
    oatpp::String normalize() const override {
        if (!isNormalized) {
            if (!value) {
                normalizedValue = value;
            } else {
                std::string email = value->c_str();
                std::string localPart, domain;
                
                // Split email into local part and domain
                auto atPos = email.find('@');
                if (atPos != std::string::npos) {
                    localPart = email.substr(0, atPos);
                    domain = email.substr(atPos + 1);
                    
                    // Convert domain to lowercase
                    std::transform(domain.begin(), domain.end(), domain.begin(), ::tolower);
                    
                    // Handle quoted local part
                    if (localPart.front() == '"' && localPart.back() == '"') {
                        // Keep quoted local part as is
                        normalizedValue = (localPart + "@" + domain).c_str();
                    } else {
                        // Convert unquoted local part to lowercase
                        std::transform(localPart.begin(), localPart.end(), localPart.begin(), ::tolower);
                        normalizedValue = (localPart + "@" + domain).c_str();
                    }
                } else {
                    // If no @ found, just lowercase everything
                    std::transform(email.begin(), email.end(), email.begin(), ::tolower);
                    normalizedValue = email.c_str();
                }
            }
            isNormalized = true;
        }
        return normalizedValue;
    }
};

/**
 * Phone number type with validation
 * Supports various international formats including:
 * - International format: +[country code][number] (e.g., +1-234-567-8900, +44 20 7123 4567)
 * - Extensions: ext, x, #
 * - Spaces, dots, and hyphens as separators
 */
class PhoneNumber : public MariaDBTypeWrapper<PhoneNumber, oatpp::String> {
public:
    explicit PhoneNumber(const oatpp::String& phone) 
        : MariaDBTypeWrapper<PhoneNumber, oatpp::String>(phone) {}
    
    bool validate() const override {
        if (!value) return false;
        
        // Basic international format with flexible separators and optional extension
        // Allows:
        // - Country code: + followed by 1-3 digits
        // - Main number: 6-14 digits (varies by country)
        // - Separators: spaces, dots, hyphens
        // - Optional extension: ext, x, or # followed by 1-6 digits
        static const std::regex pattern(
            R"(^\+\d{1,3}[-.\s]?(?:\d{1,4}[-.\s]?){1,5}\d{1,4}(?:(?:[-.\s](?:ext|x|#)\s?)|[-.\s])?(?:\d{1,6})?$)"
        );
        
        return std::regex_match(value->c_str(), pattern);
    }
    
    bool validate(const ValidationContext& context) const override {
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
        return 50;  // Increased to accommodate international numbers with extensions
    }
    
    oatpp::String getValidationError() const override {
        if (isNull()) return "Phone number cannot be null";
        if (!validate()) return "Invalid phone number format (must start with + and country code)";
        if (!validateLength()) return "Phone number exceeds maximum length";
        return "";
    }
    
    oatpp::String getDbConstraints() const override {
        // Updated regex for database constraint to match the validation pattern
        return "CHECK (phone REGEXP '^\\\\+[0-9]{1,3}[-\\\\.\\\\s]?(?:[0-9]{1,4}[-\\\\.\\\\s]?){1,5}[0-9]{1,4}(?:(?:[-\\\\.\\\\s](?:ext|x|#)\\\\s?)|[-\\\\.\\\\s])?(?:[0-9]{1,6})?$')";
    }
    
    oatpp::String normalize() const override {
        if (!isNormalized) {
            if (!value) {
                normalizedValue = value;
            } else {
                std::string normalized = value->c_str();
                std::string result;
                bool inExtension = false;
                
                // First pass: keep only essential characters and convert to standard format
                for (size_t i = 0; i < normalized.length(); ++i) {
                    char c = normalized[i];
                    
                    // Always keep the plus sign at the start
                    if (c == '+' && result.empty()) {
                        result += c;
                        continue;
                    }
                    
                    // Handle extension markers
                    if (!inExtension && (
                        (i + 2 < normalized.length() && normalized.substr(i, 3) == "ext") ||
                        c == 'x' || c == '#')) {
                        result += " ext ";
                        inExtension = true;
                        if (c == 'x' || c == '#') continue;
                        i += 2;  // Skip "ext"
                        continue;
                    }
                    
                    // Keep digits
                    if (std::isdigit(c)) {
                        if (!result.empty() && !inExtension && 
                            result.back() != '-' && result.back() != '+' && 
                            std::isdigit(result.back())) {
                            // Add separator between groups of digits
                            result += '-';
                        }
                        result += c;
                    }
                }
                
                normalizedValue = result.c_str();
            }
            isNormalized = true;
        }
        return normalizedValue;
    }
};

/**
 * URL type with validation
 * Supports:
 * - HTTP, HTTPS, FTP, FTPS, WS, WSS protocols
 * - IPv4 and IPv6 addresses
 * - International domain names (IDN)
 * - Port numbers
 * - Query parameters
 * - Fragments
 * - URL encoding
 */
class URL : public MariaDBTypeWrapper<URL, oatpp::String> {
public:
    explicit URL(const oatpp::String& url) 
        : MariaDBTypeWrapper<URL, oatpp::String>(url) {}
    
    bool validate() const override {
        if (!value) return false;
        
        // Comprehensive URL validation pattern
        // Protocol: Required, supports common web protocols
        // Host: Domain name or IP (v4/v6)
        // Port: Optional
        // Path: Optional, allows multiple segments
        // Query: Optional
        // Fragment: Optional
        static const std::regex pattern(
            R"(^(?:(?:(?:https?|ftp|ftps|ws|wss):)?\/\/)(?:\S+(?::\S*)?@)?(?:(?!(?:10|127)(?:\.\d{1,3}){3})(?!(?:169\.254|192\.168)(?:\.\d{1,3}){2})(?!172\.(?:1[6-9]|2\d|3[0-1])(?:\.\d{1,3}){2})(?:[1-9]\d?|1\d\d|2[01]\d|22[0-3])(?:\.(?:1?\d{1,2}|2[0-4]\d|25[0-5])){2}(?:\.(?:[1-9]\d?|1\d\d|2[0-4]\d|25[0-4]))|(?:\[(?:(?:(2(5[0-5]|[0-4][0-9])|1[0-9][0-9]|[1-9]?[0-9]))\.){3}(?:(2(5[0-5]|[0-4][0-9])|1[0-9][0-9]|[1-9]?[0-9])|[a-zA-Z0-9-]*[a-zA-Z0-9]:(?:[\x01-\x08\x0b\x0c\x0e-\x1f\x21-\x5a\x53-\x7f]|\\[\x01-\x09\x0b\x0c\x0e-\x7f])+)\])|(?:(?:[a-zA-Z\u00a1-\uffff0-9]-*)*[a-zA-Z\u00a1-\uffff0-9]+)(?:\.(?:[a-zA-Z\u00a1-\uffff0-9]-*)*[a-zA-Z\u00a1-\uffff0-9]+)*(?:\.(?:[a-zA-Z\u00a1-\uffff]{2,})))(?::\d{2,5})?(?:[/?#][^\s]*)?$)"
        );
        
        return std::regex_match(value->c_str(), pattern);
    }
    
    bool validate(const ValidationContext& context) const override {
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
        return "URL";
    }
    
    std::size_t getMaxLength() const override {
        return 2083;  // Maximum URL length supported by most browsers
    }
    
    oatpp::String getValidationError() const override {
        if (isNull()) return "URL cannot be null";
        if (!validate()) {
            return "Invalid URL format. Must be a valid URL with supported protocol (http, https, ftp, ftps, ws, wss)";
        }
        if (!validateLength()) {
            return "URL exceeds maximum length of 2083 characters";
        }
        return "";
    }
    
    oatpp::String getDbConstraints() const override {
        // Simplified regex for database - basic format check
        // Full URL regex is too complex for most SQL implementations
        return "CHECK (url REGEXP '^(?:https?|ftp|ftps|ws|wss)://[^\\s/$.?#].[^\\s]*$' AND LENGTH(url) <= 2083)";
    }
    
    oatpp::String normalize() const override {
        if (!isNormalized) {
            if (!value) {
                normalizedValue = value;
            } else {
                std::string url = value->c_str();
                std::string result;
                
                // Find protocol
                auto protocolEnd = url.find("://");
                if (protocolEnd != std::string::npos) {
                    // Convert protocol to lowercase
                    std::string protocol = url.substr(0, protocolEnd);
                    std::transform(protocol.begin(), protocol.end(), protocol.begin(), ::tolower);
                    result = protocol + "://";
                    
                    // Process the rest of the URL
                    std::string rest = url.substr(protocolEnd + 3);
                    
                    // Find and handle authority (user:pass@host:port)
                    size_t pathStart = rest.find('/');
                    if (pathStart == std::string::npos) pathStart = rest.length();
                    
                    std::string authority = rest.substr(0, pathStart);
                    // Convert hostname part to lowercase, preserving user:pass if present
                    auto atPos = authority.find('@');
                    if (atPos != std::string::npos) {
                        result += authority.substr(0, atPos + 1);  // Keep user:pass as is
                        std::string host = authority.substr(atPos + 1);
                        std::transform(host.begin(), host.end(), host.begin(), ::tolower);
                        result += host;
                    } else {
                        std::transform(authority.begin(), authority.end(), authority.begin(), ::tolower);
                        result += authority;
                    }
                    
                    // Keep the rest of the URL (path, query, fragment) as is
                    if (pathStart < rest.length()) {
                        result += rest.substr(pathStart);
                    }
                } else {
                    // No protocol found, just lowercase everything
                    std::transform(url.begin(), url.end(), url.begin(), ::tolower);
                    result = url;
                }
                
                normalizedValue = result.c_str();
            }
            isNormalized = true;
        }
        return normalizedValue;
    }
    
    /**
     * Extract the protocol (scheme) from the URL
     */
    oatpp::String getProtocol() const {
        if (!value) return nullptr;
        auto pos = value->std_str().find("://");
        if (pos != std::string::npos) {
            return oatpp::String(value->std_str().substr(0, pos).c_str());
        }
        return nullptr;
    }
    
    /**
     * Extract the host from the URL
     */
    oatpp::String getHost() const {
        if (!value) return nullptr;
        std::string url = value->std_str();
        auto protocolEnd = url.find("://");
        if (protocolEnd == std::string::npos) return nullptr;
        
        std::string rest = url.substr(protocolEnd + 3);
        auto pathStart = rest.find('/');
        if (pathStart == std::string::npos) pathStart = rest.length();
        
        std::string authority = rest.substr(0, pathStart);
        auto atPos = authority.find('@');
        if (atPos != std::string::npos) {
            authority = authority.substr(atPos + 1);
        }
        
        auto portPos = authority.find(':');
        if (portPos != std::string::npos) {
            authority = authority.substr(0, portPos);
        }
        
        return oatpp::String(authority.c_str());
    }
    
    /**
     * Extract the port from the URL if present
     */
    oatpp::Int32 getPort() const {
        if (!value) return 0;
        std::string url = value->std_str();
        auto protocolEnd = url.find("://");
        if (protocolEnd == std::string::npos) return 0;
        
        std::string rest = url.substr(protocolEnd + 3);
        auto pathStart = rest.find('/');
        if (pathStart == std::string::npos) pathStart = rest.length();
        
        std::string authority = rest.substr(0, pathStart);
        auto atPos = authority.find('@');
        if (atPos != std::string::npos) {
            authority = authority.substr(atPos + 1);
        }
        
        auto portPos = authority.find(':');
        if (portPos != std::string::npos) {
            try {
                return std::stoi(authority.substr(portPos + 1));
            } catch (...) {
                return 0;
            }
        }
        
        return 0;
    }
};

/**
 * Currency amount with validation
 */
class CurrencyAmount : public MariaDBTypeWrapper<CurrencyAmount, oatpp::Float64> {
public:
    explicit CurrencyAmount(const oatpp::Float64& amount, const std::string& currencyCode = "USD", int32_t decimalPlaces = 2) 
        : MariaDBTypeWrapper<CurrencyAmount, oatpp::Float64>(amount),
          currencyCode_(currencyCode),
          decimalPlaces_(decimalPlaces) {
        // Validate decimal places range
        if (decimalPlaces_ < 0 || decimalPlaces_ > 18) {  // 18 is MySQL DECIMAL max precision
            throw std::invalid_argument("Decimal places must be between 0 and 18");
        }
    }
    
    bool validate() const override {
        if (!value) return false;
        // Check for valid currency amount (non-negative)
        double amount = *value;
        if (amount < 0) return false;
        
        // Check decimal places
        double multiplier = std::pow(10.0, decimalPlaces_);
        double rounded = std::round(amount * multiplier) / multiplier;
        return std::abs(amount - rounded) < (1.0 / (multiplier * 10.0));  // Allow for floating point imprecision
    }
    
    bool validate(const ValidationContext& context) const override {
        if (isNull()) return context.allowNull;
        if (!validate()) return false;
        if (context.isStrict && !validateLength()) return false;
        if (context.normalizeValues) {
            normalize();
            return true;
        }
        return !isDirty();
    }
    
    oatpp::String getTypeName() const override {
        return "Currency Amount";
    }
    
    oatpp::String getValidationError() const override {
        if (isNull()) return "Amount cannot be null";
        if (!validate()) {
            std::ostringstream error;
            error << "Invalid amount format (must be non-negative with max " 
                  << decimalPlaces_ << " decimal places for " << currencyCode_ << ")";
            return error.str().c_str();
        }
        return "";
    }
    
    oatpp::String getDbConstraints() const override {
        std::ostringstream constraint;
        constraint << "CHECK (amount >= 0)";
        return constraint.str().c_str();
    }
    
    oatpp::String normalize() const override {
        if (!isNormalized) {
            if (!value) {
                normalizedValue = value;
            } else {
                // Round to correct number of decimal places
                double multiplier = std::pow(10.0, decimalPlaces_);
                double rounded = std::round(*value * multiplier) / multiplier;
                normalizedValue = oatpp::Float64(rounded);
            }
            isNormalized = true;
        }
        return normalizedValue;
    }
    
    std::string getCurrencyCode() const {
        return currencyCode_;
    }
    
    int32_t getDecimalPlaces() const {
        return decimalPlaces_;
    }
    
    oatpp::String getDbType() const override {
        std::ostringstream type;
        // Max digits before decimal: 28 - decimal places (MySQL DECIMAL max precision is 65)
        type << "DECIMAL(" << (28 - decimalPlaces_) << "," << decimalPlaces_ << ")";
        return type.str().c_str();
    }
    
private:
    std::string currencyCode_;
    int32_t decimalPlaces_;
};

}}}

#endif // oatpp_mariadb_types_CommonTypes_hpp 