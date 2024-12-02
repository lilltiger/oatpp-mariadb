#ifndef oatpp_mariadb_types_Status_hpp
#define oatpp_mariadb_types_Status_hpp

#include "MariaDBTypeWrapper.hpp"
#include <unordered_map>
#include <unordered_set>

namespace oatpp { namespace mariadb { namespace types {

/**
 * Status type wrapper for handling predefined status values with state transition validation.
 * Extends MariaDBTypeWrapper to provide database integration with ENUM type.
 */
class Status : public MariaDBTypeWrapper<Status, oatpp::String> {
private:
    // Predefined status values and their allowed transitions
    static std::unordered_map<std::string, std::unordered_set<std::string>> transitions;
    
    // All valid status values
    static std::unordered_set<std::string> validValues;
    
    // Current status value
    mutable std::string currentValue;
    
    // Previous status value (for transition validation)
    mutable std::string previousValue;

public:
    /**
     * Constructor
     * @param value - Initial status value
     */
    explicit Status(const oatpp::String& value) : MariaDBTypeWrapper(value) {
        if (value) {
            currentValue = std::string(value->c_str());
        }
    }
    
    /**
     * Add a valid status value
     * @param status - The status value to add
     */
    static void addValidStatus(const std::string& status) {
        validValues.insert(status);
    }
    
    /**
     * Add a valid transition between statuses
     * @param fromStatus - The starting status
     * @param toStatus - The target status
     */
    static void addTransition(const std::string& fromStatus, const std::string& toStatus) {
        transitions[fromStatus].insert(toStatus);
        // Ensure both statuses are in validValues
        validValues.insert(fromStatus);
        validValues.insert(toStatus);
    }
    
    /**
     * Check if a transition is valid
     * @param fromStatus - The starting status
     * @param toStatus - The target status
     * @return true if the transition is valid
     */
    static bool isValidTransition(const std::string& fromStatus, const std::string& toStatus) {
        if (transitions.find(fromStatus) == transitions.end()) {
            return false;
        }
        return transitions[fromStatus].find(toStatus) != transitions[fromStatus].end();
    }
    
    /**
     * Get all valid status values
     * @return Set of valid status values
     */
    static const std::unordered_set<std::string>& getValidValues() {
        return validValues;
    }
    
    /**
     * Get allowed transitions for a status
     * @param status - The status to get transitions for
     * @return Set of allowed target statuses
     */
    static const std::unordered_set<std::string>& getAllowedTransitions(const std::string& status) {
        static const std::unordered_set<std::string> empty;
        auto it = transitions.find(status);
        return it != transitions.end() ? it->second : empty;
    }
    
    /**
     * Validate the status value
     * @return true if valid
     */
    bool validate() const override {
        if (!value) {
            return true; // Null is valid unless restricted by context
        }
        std::string val = std::string(value->c_str());
        return validValues.find(val) != validValues.end();
    }
    
    /**
     * Validate with context
     * @param context - Validation context
     * @return true if valid
     */
    bool validate(const ValidationContext& context) const override {
        if (!value) {
            return context.allowNull;
        }
        
        if (!validate()) {
            return false;
        }
        
        // If we're in strict mode, validate the transition
        if (context.isStrict && !previousValue.empty()) {
            return isValidTransition(previousValue, currentValue);
        }
        
        return true;
    }
    
    /**
     * Get validation error message
     * @return Error message
     */
    oatpp::String getValidationError() const override {
        if (!value) {
            return "Status cannot be null";
        }
        
        std::string val = std::string(value->c_str());
        if (validValues.find(val) == validValues.end()) {
            return "Invalid status value: " + val;
        }
        
        if (!previousValue.empty() && !isValidTransition(previousValue, val)) {
            return "Invalid status transition from '" + previousValue + "' to '" + val + "'";
        }
        
        return nullptr;
    }
    
    /**
     * Get type name
     * @return Type name
     */
    oatpp::String getTypeName() const override {
        return "Status";
    }
    
    /**
     * Get database type
     * @return Database type definition
     */
    oatpp::String getDbType() const override {
        std::ostringstream oss;
        oss << "ENUM(";
        bool first = true;
        for (const auto& status : validValues) {
            if (!first) {
                oss << ",";
            }
            oss << "'" << status << "'";
            first = false;
        }
        oss << ")";
        return oatpp::String(oss.str().c_str());
    }
    
    /**
     * Get database constraints
     * @return Database constraints
     */
    oatpp::String getDbConstraints() const override {
        return "NOT NULL";  // ENUMs are typically NOT NULL
    }
    
    /**
     * Update status value
     * @param newValue - New status value
     * @return true if update was successful
     */
    bool updateStatus(const oatpp::String& newValue) {
        if (!newValue) {
            return false;
        }
        
        std::string newVal = std::string(newValue->c_str());
        if (validValues.find(newVal) == validValues.end()) {
            return false;
        }
        
        if (!currentValue.empty() && !isValidTransition(currentValue, newVal)) {
            return false;
        }
        
        previousValue = currentValue;
        currentValue = newVal;
        value = newValue;
        return true;
    }
    
    /**
     * Get current status value
     * @return Current status
     */
    const std::string& getCurrentStatus() const {
        return currentValue;
    }
    
    /**
     * Get previous status value
     * @return Previous status
     */
    const std::string& getPreviousStatus() const {
        return previousValue;
    }
};

}}} // namespace oatpp::mariadb::types

#endif // oatpp_mariadb_types_Status_hpp 