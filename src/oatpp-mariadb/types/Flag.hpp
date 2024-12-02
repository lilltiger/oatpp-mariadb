#ifndef oatpp_mariadb_types_Flag_hpp
#define oatpp_mariadb_types_Flag_hpp

#include "MariaDBTypeWrapper.hpp"
#include <unordered_map>
#include <vector>
#include <string>

namespace oatpp { namespace mariadb { namespace types {

/**
 * Flag type for MariaDB.
 * Represents a bitfield stored as BIGINT UNSIGNED.
 */
class Flag : public MariaDBTypeWrapper<Flag, oatpp::UInt64> {
private:
    static std::unordered_map<std::string, v_uint64> namedFlags;
    static std::unordered_map<v_uint64, std::string> flagNames;
    
public:
    /**
     * Constructor.
     * @param value - UInt64 value.
     */
    explicit Flag(const oatpp::UInt64& value = oatpp::UInt64((v_uint64)0)) 
        : MariaDBTypeWrapper<Flag, oatpp::UInt64>(value) {}
    
    /**
     * Constructor from flag name.
     * @param flagName - Name of the flag.
     */
    explicit Flag(const std::string& flagName);
    
    /**
     * Register a named flag.
     * @param name - Name of the flag.
     * @param value - Value of the flag.
     */
    static void registerFlag(const std::string& name, v_uint64 value);
    
    /**
     * Get flag name for a value.
     * @param value - Flag value.
     * @return Name of the flag.
     */
    static std::string getFlagName(v_uint64 value);
    
    /**
     * Get flag value for a name.
     * @param name - Name of the flag.
     * @return Value of the flag.
     */
    static v_uint64 getFlagValue(const std::string& name);
    
    /**
     * Check if a flag is set.
     * @param flag - Flag to check.
     * @return true if flag is set.
     */
    template<typename T>
    bool hasFlag(const T& flag) const;
    
    /**
     * Set a flag.
     * @param flag - Flag to set.
     */
    template<typename T>
    void setFlag(const T& flag);
    
    /**
     * Clear a flag.
     * @param flag - Flag to clear.
     */
    template<typename T>
    void clearFlag(const T& flag);
    
    /**
     * Toggle a flag.
     * @param flag - Flag to toggle.
     */
    template<typename T>
    void toggleFlag(const T& flag);
    
    /**
     * Get list of set flag names.
     * @return Vector of flag names.
     */
    std::vector<std::string> getSetFlags() const;
    
    /**
     * Convert flags to string representation.
     * @return String representation of flags.
     */
    std::string toString() const;
    
    /**
     * Create Flag from string representation.
     * @param str - String representation.
     * @return Flag instance.
     */
    static Flag fromString(const std::string& str);
    
    /**
     * Get database type.
     * @return String representation of database type.
     */
    oatpp::String getDbType() const override;
    
    /**
     * Get type name.
     * @return String representation of type name.
     */
    oatpp::String getTypeName() const override;
    
    /**
     * Validate the flag value.
     * @return true if valid.
     */
    bool validate() const override;
    
    /**
     * Validate the flag value with context.
     * @param context - Validation context.
     * @return true if valid.
     */
    bool validate(const ValidationContext& context) const override;
    
    /**
     * Get validation error message.
     * @return Error message.
     */
    oatpp::String getValidationError() const override;
    
    /**
     * Validate length.
     * @return true if length is valid.
     */
    bool validateLength() const override {
        return true; // Numeric types don't need length validation
    }
    
    /**
     * Get maximum length.
     * @return Maximum length.
     */
    std::size_t getMaxLength() const override {
        return sizeof(v_uint64); // Size of underlying type
    }
};

}}} // namespace oatpp::mariadb::types

#endif // oatpp_mariadb_types_Flag_hpp 