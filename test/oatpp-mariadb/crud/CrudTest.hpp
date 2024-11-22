#ifndef oatpp_test_mariadb_crud_CrudTest_hpp
#define oatpp_test_mariadb_crud_CrudTest_hpp

#include "oatpp-test/UnitTest.hpp"

namespace oatpp { namespace test { namespace mariadb { namespace crud {

class CrudTest : public oatpp::test::UnitTest {
public:
  CrudTest() : UnitTest("TEST[mariadb::crud::CrudTest]") {}
  ~CrudTest() override = default;
  void onRun() override;
};

}}}}

#endif // oatpp_test_mariadb_crud_CrudTest_hpp
