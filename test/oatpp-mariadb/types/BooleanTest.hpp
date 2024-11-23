#ifndef oatpp_test_mariadb_types_BooleanTest_hpp
#define oatpp_test_mariadb_types_BooleanTest_hpp

#include "oatpp-test/UnitTest.hpp"

namespace oatpp { namespace test { namespace mariadb { namespace types {

class BooleanTest : public UnitTest {
public:
  BooleanTest() : UnitTest("TEST[mariadb::types::BooleanTest]") {}
  void onRun() override;
};

}}}}

#endif // oatpp_test_mariadb_types_BooleanTest_hpp
