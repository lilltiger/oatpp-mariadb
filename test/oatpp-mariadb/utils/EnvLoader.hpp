#ifndef oatpp_test_mariadb_utils_EnvLoader_hpp
#define oatpp_test_mariadb_utils_EnvLoader_hpp

#include "oatpp/core/base/Environment.hpp"
#include <string>
#include <fstream>
#include <unordered_map>
#include <vector>

namespace oatpp { namespace test { namespace mariadb { namespace utils {

class EnvLoader {
private:
    std::unordered_map<std::string, std::string> envVars;

    void loadFromFile(const std::string& path) {
        OATPP_LOGD("EnvLoader", "Attempting to load .env file from path: %s", path.c_str());
        std::ifstream file(path);
        if (!file.is_open()) {
            OATPP_LOGD("EnvLoader", "Failed to open .env file at path: %s", path.c_str());
            return;
        }
        std::string line;
        
        while (std::getline(file, line)) {
            // Skip empty lines and comments
            if (line.empty() || line[0] == '#') continue;
            
            // Find the equals sign
            auto pos = line.find('=');
            if (pos != std::string::npos) {
                std::string key = line.substr(0, pos);
                std::string value = line.substr(pos + 1);
                
                // Remove quotes if present
                if (value.size() >= 2 && (value[0] == '"' || value[0] == '\'')) {
                    value = value.substr(1, value.size() - 2);
                }
                
                envVars[key] = value;
                OATPP_LOGD("EnvLoader", "Loaded env var: %s = %s", key.c_str(), value.c_str());
            }
        }
    }

public:
    EnvLoader() {
        // Try multiple possible locations for the .env file
        const std::vector<std::string> paths = {
            ".env",                  // Current directory
            "../.env",              // One level up
            "../../.env",           // Two levels up
            "../../../.env",        // Three levels up
            "test/.env",           // test directory
            "../test/.env"         // From build directory
        };
        
        for (const auto& path : paths) {
            OATPP_LOGD("EnvLoader", "Trying path: %s", path.c_str());
            std::ifstream file(path);
            if (file.good()) {
                OATPP_LOGD("EnvLoader", "Found .env file at: %s", path.c_str());
                loadFromFile(path);
                break;
            }
        }
    }
    
    std::string get(const std::string& key, const std::string& defaultValue = "") const {
        auto it = envVars.find(key);
        return it != envVars.end() ? it->second : defaultValue;
    }
    
    int getInt(const std::string& key, int defaultValue = 0) const {
        auto it = envVars.find(key);
        return it != envVars.end() ? std::stoi(it->second) : defaultValue;
    }
};

}}}}

#endif // oatpp_test_mariadb_utils_EnvLoader_hpp
