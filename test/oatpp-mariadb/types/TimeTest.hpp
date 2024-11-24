#ifndef oatpp_test_mariadb_types_TimeTest_hpp
#define oatpp_test_mariadb_types_TimeTest_hpp

#include "oatpp-test/UnitTest.hpp"

namespace oatpp { namespace test { namespace mariadb { namespace types {

class TimeTest : public oatpp::test::UnitTest {
public:
  TimeTest() : UnitTest("TEST[mariadb::types::TimeTest]") {}
  void onRun() override;
};

}}}}

#endif // oatpp_test_mariadb_types_TimeTest_hpp
