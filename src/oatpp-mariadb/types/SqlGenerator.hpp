#ifndef oatpp_mariadb_types_SqlGenerator_hpp
#define oatpp_mariadb_types_SqlGenerator_hpp

#include "TypeWrapper.hpp"
#include <sstream>

namespace oatpp { namespace mariadb { namespace types {

/**
 * Helper class for generating SQL definitions from type wrappers
 */
class SqlGenerator {
public:
    /**
     * Generate complete SQL column definition
     * @param fieldName - Name of the field
     * @param wrapper - Type wrapper instance
     * @return Complete SQL column definition
     */
    template<typename T, typename U>
    static oatpp::String generateColumnDef(const oatpp::String& fieldName, const TypeWrapper<T, U>& wrapper) {
        std::stringstream ss;
        ss << fieldName->c_str() << " " << wrapper.getSqlType()->c_str();
        
        if (!wrapper.isNullable()) {
            ss << " NOT NULL";
        }
        
        auto constraints = wrapper.getDbConstraints();
        if (constraints && !constraints->empty()) {
            ss << " " << constraints->c_str();
        }
        
        auto additional = wrapper.getAdditionalConstraints();
        if (additional && !additional->empty()) {
            ss << " " << additional->c_str();
        }
        
        return ss.str();
    }
    
    /**
     * Generate SQL for table creation
     * @param tableName - Name of the table
     * @param columns - Vector of column definitions
     * @return Complete CREATE TABLE statement
     */
    static oatpp::String generateCreateTable(const oatpp::String& tableName,
                                           const std::vector<oatpp::String>& columns) {
        std::stringstream ss;
        ss << "CREATE TABLE IF NOT EXISTS " << tableName->c_str() << " (";
        
        // Add id column by default
        ss << "\n  id INTEGER PRIMARY KEY AUTO_INCREMENT,";
        
        // Add other columns
        for (const auto& col : columns) {
            ss << "\n  " << col->c_str() << ",";
        }
        
        // Remove trailing comma and close
        std::string result = ss.str();
        result = result.substr(0, result.length() - 1);
        result += "\n) ENGINE=InnoDB;";
        
        return result;
    }
    
    /**
     * Generate parameter placeholder for prepared statements
     * @param wrapper - Type wrapper instance
     * @return Parameter placeholder
     */
    template<typename T, typename U>
    static oatpp::String generateParamPlaceholder(const TypeWrapper<T, U>& wrapper) {
        return "?";
    }
};

}}}

#endif // oatpp_mariadb_types_SqlGenerator_hpp 