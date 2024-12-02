#ifndef oatpp_mariadb_types_Flag_hpp
#define oatpp_mariadb_types_Flag_hpp

#include "MariaDBTypeWrapper.hpp"
#include <unordered_map>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>

namespace oatpp { namespace mariadb { namespace types {

/**
 * Flag type for MariaDB.
 * Represents a bitfield stored as BIT(N).
 * @tparam N Number of bits (1-64). Default is 64.
 */
template<size_t N = 64>
class Flag : public MariaDBTypeWrapper<Flag<N>, oatpp::UInt64> {
private:
    static_assert(N > 0 && N <= 64, "Flag bit size must be between 1 and 64");
    static std::unordered_map<std::string, v_uint64> namedFlags;
    static std::unordered_map<v_uint64, std::string> flagNames;
    static constexpr v_uint64 MAX_VALUE = (N == 64) ? ~0ULL : ((1ULL << N) - 1);
    
public:
    /**
     * Constructor.
     * @param value - UInt64 value.
     */
    explicit Flag(const oatpp::UInt64& value = oatpp::UInt64((v_uint64)0)) 
        : MariaDBTypeWrapper<Flag<N>, oatpp::UInt64>(value) {}
    
    /**
     * Constructor from flag name.
     * @param flagName - Name of the flag.
     */
    explicit Flag(const std::string& flagName) 
        : MariaDBTypeWrapper<Flag<N>, oatpp::UInt64>(oatpp::UInt64((v_uint64)getFlagValue(flagName))) {}
    
    /**
     * Register a named flag.
     * @param name - Name of the flag.
     * @param value - Value of the flag.
     */
    static void registerFlag(const std::string& name, v_uint64 value) {
        if (value > MAX_VALUE) {
            throw std::runtime_error("Flag value exceeds maximum for " + std::to_string(N) + " bits");
        }
        namedFlags[name] = value;
        flagNames[value] = name;
    }
    
    /**
     * Get flag name for a value.
     * @param value - Flag value.
     * @return Name of the flag.
     */
    static std::string getFlagName(v_uint64 value) {
        auto it = flagNames.find(value);
        return it != flagNames.end() ? it->second : "";
    }
    
    /**
     * Get flag value for a name.
     * @param name - Name of the flag.
     * @return Value of the flag.
     */
    static v_uint64 getFlagValue(const std::string& name) {
        auto it = namedFlags.find(name);
        return it != namedFlags.end() ? it->second : (v_uint64)0;
    }
    
    /**
     * Check if a flag is set.
     * @param flag - Flag to check.
     * @return true if flag is set.
     */
    bool hasFlag(const v_uint64& flag) const {
        if (flag > MAX_VALUE) {
            throw std::runtime_error("Flag value exceeds maximum for " + std::to_string(N) + " bits");
        }
        return (this->value.getValue(0) & flag) == flag;
    }
    
    bool hasFlag(const std::string& flag) const {
        return hasFlag(getFlagValue(flag));
    }
    
    /**
     * Set a flag.
     * @param flag - Flag to set.
     */
    void setFlag(const v_uint64& flag) {
        if (flag > MAX_VALUE) {
            throw std::runtime_error("Flag value exceeds maximum for " + std::to_string(N) + " bits");
        }
        if (this->value) {
            this->value = oatpp::UInt64(this->value.getValue(0) | flag);
        } else {
            this->value = oatpp::UInt64(flag);
        }
    }
    
    void setFlag(const std::string& flag) {
        setFlag(getFlagValue(flag));
    }
    
    /**
     * Clear a flag.
     * @param flag - Flag to clear.
     */
    void clearFlag(const v_uint64& flag) {
        if (flag > MAX_VALUE) {
            throw std::runtime_error("Flag value exceeds maximum for " + std::to_string(N) + " bits");
        }
        if (this->value) {
            this->value = oatpp::UInt64(this->value.getValue(0) & ~flag);
        }
    }
    
    void clearFlag(const std::string& flag) {
        clearFlag(getFlagValue(flag));
    }
    
    /**
     * Toggle a flag.
     * @param flag - Flag to toggle.
     */
    void toggleFlag(const v_uint64& flag) {
        if (flag > MAX_VALUE) {
            throw std::runtime_error("Flag value exceeds maximum for " + std::to_string(N) + " bits");
        }
        if (this->value) {
            this->value = oatpp::UInt64(this->value.getValue(0) ^ flag);
        } else {
            this->value = oatpp::UInt64(flag);
        }
    }
    
    void toggleFlag(const std::string& flag) {
        toggleFlag(getFlagValue(flag));
    }
    
    /**
     * Get list of set flag names.
     * @return Vector of flag names.
     */
    std::vector<std::string> getSetFlags() const {
        std::vector<std::string> result;
        if (!this->value) return result;
        
        v_uint64 val = this->value.getValue(0);
        std::vector<std::pair<v_uint64, std::string>> sortedFlags;
        
        for (const auto& pair : namedFlags) {
            if ((val & pair.second) == pair.second) {
                sortedFlags.push_back({pair.second, pair.first});
            }
        }
        
        std::sort(sortedFlags.begin(), sortedFlags.end());
        
        result.reserve(sortedFlags.size());
        for (const auto& pair : sortedFlags) {
            result.push_back(pair.second);
        }
        
        return result;
    }
    
    /**
     * Convert flags to string representation.
     * @return String representation of flags.
     */
    std::string toString() const {
        auto flags = getSetFlags();
        if (flags.empty()) {
            return "0";
        }
        
        std::ostringstream oss;
        for (size_t i = 0; i < flags.size(); ++i) {
            if (i > 0) {
                oss << "|";
            }
            oss << flags[i];
        }
        return oss.str();
    }
    
    /**
     * Create Flag from string representation.
     * @param str - String representation.
     * @return Flag instance.
     */
    static Flag fromString(const std::string& str) {
        if (str == "0") {
            return Flag();
        }
        
        v_uint64 result = 0;
        std::istringstream iss(str);
        std::string token;
        
        while (std::getline(iss, token, '|')) {
            result |= getFlagValue(token);
        }
        
        if (result > MAX_VALUE) {
            throw std::runtime_error("Combined flag value exceeds maximum for " + std::to_string(N) + " bits");
        }
        
        return Flag(oatpp::UInt64(result));
    }
    
    /**
     * Get database type.
     * @return String representation of database type.
     */
    oatpp::String getDbType() const override {
        return oatpp::String(std::string("BIT(") + std::to_string(N) + ")");
    }
    
    /**
     * Get type name.
     * @return String representation of type name.
     */
    oatpp::String getTypeName() const override {
        return oatpp::String("Flag<" + std::to_string(N) + ">");
    }
    
    /**
     * Validate the flag value.
     * @return true if valid.
     */
    bool validate() const override {
        if (!this->value) return true;
        return this->value.getValue(0) <= MAX_VALUE;
    }
    
    /**
     * Validate the flag value with context.
     * @param context - Validation context.
     * @return true if valid.
     */
    bool validate(const ValidationContext& context) const override {
        return validate();
    }
    
    /**
     * Get validation error message.
     * @return Error message.
     */
    oatpp::String getValidationError() const override {
        return oatpp::String("Flag value exceeds maximum for " + std::to_string(N) + " bits");
    }
    
    bool validateLength() const override {
        return true; // Flags don't need length validation
    }
};

// Static member initialization
template<size_t N>
std::unordered_map<std::string, v_uint64> Flag<N>::namedFlags;

template<size_t N>
std::unordered_map<v_uint64, std::string> Flag<N>::flagNames;

}}} // namespace oatpp::mariadb::types

#endif // oatpp_mariadb_types_Flag_hpp 