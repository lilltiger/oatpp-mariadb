#include "oatpp-mariadb/ql_template/ParserTest.hpp"
#include "oatpp-mariadb/types/NumericTest.hpp"
#include "oatpp-mariadb/types/Int64Test.hpp"
#include "oatpp-mariadb/types/Float64Test.hpp"
#include "oatpp-mariadb/types/UInt8Test.hpp"
#include "oatpp-mariadb/types/DateTest.hpp"
#include "oatpp-mariadb/types/DateTimeTest.hpp"
#include "oatpp-mariadb/types/VarCharTest.hpp"
#include "oatpp-mariadb/types/BooleanTest.hpp"
#include "oatpp-mariadb/types/ReturningTest.hpp"
#include "oatpp-mariadb/types/TimeTest.hpp"
#include "oatpp-mariadb/types/YearTest.hpp"
#include "oatpp-mariadb/types/EnumTest.hpp"
#include "oatpp-mariadb/types/SetTest.hpp"
#include "oatpp-mariadb/types/JsonTest.hpp"
#include "oatpp-mariadb/types/BinaryTest.hpp"
#include "oatpp-mariadb/types/Int32Test.hpp"
#include "oatpp-mariadb/types/StringTest.hpp"
#include "oatpp-mariadb/types/AnyTypeTest.hpp"
#include "oatpp-mariadb/migration/SchemaVersionTest.hpp"

#include "oatpp-mariadb/transaction/TransactionTest.hpp"
#include "oatpp-mariadb/crud/CrudTest.hpp"
#include "oatpp-mariadb/crud/ProductCrudTest.hpp"
#include "oatpp-mariadb/crud/EnhancedCrudTest.hpp"

#include "oatpp/core/base/Environment.hpp"

#include <iostream>

namespace {

void runTests() {
  OATPP_RUN_TEST(oatpp::test::mariadb::ql_template::ParserTest);
  OATPP_RUN_TEST(oatpp::test::mariadb::types::Int64Test);
  OATPP_RUN_TEST(oatpp::test::mariadb::types::Float64Test);
  OATPP_RUN_TEST(oatpp::test::mariadb::types::UInt8Test);
  OATPP_RUN_TEST(oatpp::test::mariadb::types::NumericTest);
  OATPP_RUN_TEST(oatpp::test::mariadb::types::DateTest);
  OATPP_RUN_TEST(oatpp::test::mariadb::types::DateTimeTest);
  OATPP_RUN_TEST(oatpp::test::mariadb::types::VarCharTest);
  OATPP_RUN_TEST(oatpp::test::mariadb::types::BooleanTest);
  OATPP_RUN_TEST(oatpp::test::mariadb::types::ReturningTest);
  OATPP_RUN_TEST(oatpp::test::mariadb::types::TimeTest);
  OATPP_RUN_TEST(oatpp::test::mariadb::types::YearTest);
  OATPP_RUN_TEST(oatpp::test::mariadb::types::EnumTest);
  OATPP_RUN_TEST(oatpp::test::mariadb::transaction::TransactionTest);
  OATPP_RUN_TEST(oatpp::test::mariadb::types::StringTest);
  OATPP_RUN_TEST(oatpp::test::mariadb::types::TimeTest);
  OATPP_RUN_TEST(oatpp::test::mariadb::types::YearTest);
  OATPP_RUN_TEST(oatpp::test::mariadb::types::EnumTest);
  OATPP_RUN_TEST(oatpp::test::mariadb::types::SetTest);
  OATPP_RUN_TEST(oatpp::test::mariadb::types::JsonTest);
  OATPP_RUN_TEST(oatpp::test::mariadb::types::BinaryTest);
  OATPP_RUN_TEST(oatpp::test::mariadb::types::TimeTest);
  OATPP_RUN_TEST(oatpp::test::mariadb::types::YearTest);
  OATPP_RUN_TEST(oatpp::test::mariadb::types::EnumTest);
  OATPP_RUN_TEST(oatpp::test::mariadb::transaction::TransactionTest);
  OATPP_RUN_TEST(oatpp::test::mariadb::crud::CrudTest);
  OATPP_RUN_TEST(oatpp::test::mariadb::crud::ProductCrudTest);
  OATPP_RUN_TEST(oatpp::test::mariadb::crud::EnhancedCrudTest);
  OATPP_RUN_TEST(oatpp::test::mariadb::types::AnyTypeTest);
  OATPP_RUN_TEST(oatpp::test::mariadb::migration::SchemaVersionTest);
}

}

int main() {

  oatpp::base::Environment::init();

  runTests();

  /* Print how much objects were created during app running, and what have left-probably leaked */
  /* Disable object counting for release builds using '-D OATPP_DISABLE_ENV_OBJECT_COUNTERS' flag for better performance */
  std::cout << "\nEnvironment:\n";
  std::cout << "objectsCount = " << oatpp::base::Environment::getObjectsCount() << "\n";
  std::cout << "objectsCreated = " << oatpp::base::Environment::getObjectsCreated() << "\n\n";

  OATPP_ASSERT(oatpp::base::Environment::getObjectsCount() == 0);

  oatpp::base::Environment::destroy();

  return 0;
}
