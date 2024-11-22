#ifndef oatpp_test_mariadb_crud_ProductCrudTest_hpp
#define oatpp_test_mariadb_crud_ProductCrudTest_hpp

#include "oatpp-test/UnitTest.hpp"

namespace oatpp { namespace test { namespace mariadb { namespace crud {

class ProductCrudTest : public oatpp::test::UnitTest {
public:
  ProductCrudTest() : UnitTest("TEST[mariadb::crud::ProductCrudTest]") {}
  void onRun() override;
};

}}}}

#endif // oatpp_test_mariadb_crud_ProductCrudTest_hpp
