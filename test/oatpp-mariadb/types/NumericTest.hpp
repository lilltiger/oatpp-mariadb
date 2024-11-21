#ifndef oatpp_test_mariadb_types_NumericTest_hpp
#define oatpp_test_mariadb_types_NumericTest_hpp

#include "oatpp-test/UnitTest.hpp"

namespace oatpp { namespace test { namespace mariadb { namespace types {

class NumericTest : public UnitTest {
public:
  NumericTest() : UnitTest("TEST[mariadb::types::NumericTest]") {}
  void onRun() override;
};

}}}}

#endif // oatpp_test_mariadb_types_NumericTest_hpp
