#include "Flag.hpp"
#include <sstream>
#include <algorithm>

namespace oatpp { namespace mariadb { namespace types {

std::unordered_map<std::string, v_uint64> Flag::namedFlags;
std::unordered_map<v_uint64, std::string> Flag::flagNames;

Flag::Flag(const std::string& flagName) 
    : MariaDBTypeWrapper<Flag, oatpp::UInt64>(oatpp::UInt64((v_uint64)getFlagValue(flagName))) {}

void Flag::registerFlag(const std::string& name, v_uint64 value) {
    namedFlags[name] = value;
    flagNames[value] = name;
}

std::string Flag::getFlagName(v_uint64 value) {
    auto it = flagNames.find(value);
    return it != flagNames.end() ? it->second : "";
}

v_uint64 Flag::getFlagValue(const std::string& name) {
    auto it = namedFlags.find(name);
    return it != namedFlags.end() ? it->second : (v_uint64)0;
}

template<>
bool Flag::hasFlag<v_uint64>(const v_uint64& flag) const {
    return (value.getValue(0) & flag) == flag;
}

template<>
bool Flag::hasFlag<std::string>(const std::string& flag) const {
    return hasFlag(getFlagValue(flag));
}

template<>
void Flag::setFlag<v_uint64>(const v_uint64& flag) {
    if (value) {
        value = oatpp::UInt64(value.getValue(0) | flag);
    } else {
        value = oatpp::UInt64(flag);
    }
}

template<>
void Flag::setFlag<std::string>(const std::string& flag) {
    setFlag(getFlagValue(flag));
}

template<>
void Flag::clearFlag<v_uint64>(const v_uint64& flag) {
    if (value) {
        value = oatpp::UInt64(value.getValue(0) & ~flag);
    }
}

template<>
void Flag::clearFlag<std::string>(const std::string& flag) {
    clearFlag(getFlagValue(flag));
}

template<>
void Flag::toggleFlag<v_uint64>(const v_uint64& flag) {
    if (value) {
        value = oatpp::UInt64(value.getValue(0) ^ flag);
    } else {
        value = oatpp::UInt64(flag);
    }
}

template<>
void Flag::toggleFlag<std::string>(const std::string& flag) {
    toggleFlag(getFlagValue(flag));
}

std::vector<std::string> Flag::getSetFlags() const {
    std::vector<std::string> result;
    if (!value) return result;
    
    v_uint64 val = value.getValue(0);
    for (const auto& pair : namedFlags) {
        if ((val & pair.second) == pair.second) {
            result.push_back(pair.first);
        }
    }
    
    return result;
}

std::string Flag::toString() const {
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

Flag Flag::fromString(const std::string& str) {
    if (str == "0") {
        return Flag();
    }
    
    v_uint64 result = 0;
    std::istringstream iss(str);
    std::string token;
    
    while (std::getline(iss, token, '|')) {
        result |= getFlagValue(token);
    }
    
    return Flag(oatpp::UInt64(result));
}

oatpp::String Flag::getDbType() const {
    return oatpp::String("BIGINT UNSIGNED");
}

oatpp::String Flag::getTypeName() const {
    return oatpp::String("Flag");
}

bool Flag::validate() const {
    return true; // All flag values are valid
}

bool Flag::validate(const ValidationContext& context) const {
    return validate();
}

oatpp::String Flag::getValidationError() const {
    return oatpp::String("Invalid flag value");
}

}}} // namespace oatpp::mariadb::types 