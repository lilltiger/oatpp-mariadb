#include "oatpp-mariadb/ql_template/ParserTest.hpp"
#include "oatpp-mariadb/types/NumericTest.hpp"
#include "oatpp-mariadb/types/Int64Test.hpp"
#include "oatpp-mariadb/transaction/TransactionTest.hpp"
#include "oatpp-mariadb/crud/CrudTest.hpp"
#include "oatpp-mariadb/crud/ProductCrudTest.hpp"

#include "oatpp/core/base/Environment.hpp"

#include <iostream>

namespace {

void runTests() {
  //OATPP_RUN_TEST(oatpp::test::mariadb::ql_template::ParserTest);
  OATPP_RUN_TEST(oatpp::test::mariadb::types::Int64Test);
  OATPP_RUN_TEST(oatpp::test::mariadb::types::NumericTest);
  //OATPP_RUN_TEST(oatpp::test::mariadb::transaction::TransactionTest);
  //OATPP_RUN_TEST(oatpp::test::mariadb::crud::CrudTest);
  //OATPP_RUN_TEST(oatpp::test::mariadb::crud::ProductCrudTest);
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
