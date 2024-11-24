#ifndef oatpp_test_mariadb_types_YearTest_hpp
#define oatpp_test_mariadb_types_YearTest_hpp

#include "oatpp-test/UnitTest.hpp"

namespace oatpp { namespace test { namespace mariadb { namespace types {

class YearTest : public oatpp::test::UnitTest {
public:
  YearTest() : UnitTest("TEST[mariadb::types::YearTest]") {}
  void onRun() override;
};

}}}}

#endif // oatpp_test_mariadb_types_YearTest_hpp
