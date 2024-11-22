#ifndef oatpp_test_mariadb_types_VarCharTest_hpp
#define oatpp_test_mariadb_types_VarCharTest_hpp

#include "oatpp-test/UnitTest.hpp"

namespace oatpp { namespace test { namespace mariadb { namespace types {

class VarCharTest : public oatpp::test::UnitTest {
public:
  VarCharTest() : UnitTest("TEST[mariadb::types::VarCharTest]") {}
  void onRun() override;
};

}}}}

#endif // oatpp_test_mariadb_types_VarCharTest_hpp
