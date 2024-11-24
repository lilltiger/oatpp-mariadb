#ifndef oatpp_test_mariadb_types_EnumTest_hpp
#define oatpp_test_mariadb_types_EnumTest_hpp

#include "oatpp-test/UnitTest.hpp"

namespace oatpp { namespace test { namespace mariadb { namespace types {

class EnumTest : public oatpp::test::UnitTest {
public:
  EnumTest() : UnitTest("TEST[mariadb::types::EnumTest]") {}
  void onRun() override;
};

}}}}

#endif // oatpp_test_mariadb_types_EnumTest_hpp
