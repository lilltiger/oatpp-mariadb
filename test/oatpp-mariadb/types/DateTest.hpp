#ifndef oatpp_test_mariadb_types_DateTest_hpp
#define oatpp_test_mariadb_types_DateTest_hpp

#include "oatpp-test/UnitTest.hpp"

namespace oatpp { namespace test { namespace mariadb { namespace types {

class DateTest : public oatpp::test::UnitTest {
public:
  DateTest() : UnitTest("TEST[mariadb::types::DateTest]") {}
  void onRun() override;
};

}}}}

#endif // oatpp_test_mariadb_types_DateTest_hpp
