#ifndef oatpp_test_mariadb_types_DateTimeTest_hpp
#define oatpp_test_mariadb_types_DateTimeTest_hpp

#include "oatpp-test/UnitTest.hpp"

namespace oatpp { namespace test { namespace mariadb { namespace types {

class DateTimeTest : public oatpp::test::UnitTest {
public:
  DateTimeTest() : UnitTest("TEST[mariadb::types::DateTimeTest]") {}
  void onRun() override;
};

}}}}

#endif // oatpp_test_mariadb_types_DateTimeTest_hpp
