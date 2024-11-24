#ifndef oatpp_test_mariadb_types_ReturningTest_hpp
#define oatpp_test_mariadb_types_ReturningTest_hpp

#include "oatpp-test/UnitTest.hpp"

namespace oatpp { namespace test { namespace mariadb { namespace types {

class ReturningTest : public oatpp::test::UnitTest {
public:
  ReturningTest() : UnitTest("TEST[mariadb::types::ReturningTest]") {}
  void onRun() override;
};

}}}}

#endif // oatpp_test_mariadb_types_ReturningTest_hpp
