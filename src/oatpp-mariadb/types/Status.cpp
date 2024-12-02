#include "Status.hpp"

namespace oatpp { namespace mariadb { namespace types {

std::unordered_map<std::string, std::unordered_set<std::string>> Status::transitions;
std::unordered_set<std::string> Status::validValues;

}}} // namespace oatpp::mariadb::types 