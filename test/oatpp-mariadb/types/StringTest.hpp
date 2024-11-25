#ifndef oatpp_test_mariadb_types_StringTest_hpp
#define oatpp_test_mariadb_types_StringTest_hpp

#include "oatpp-test/UnitTest.hpp"

namespace oatpp { namespace test { namespace mariadb { namespace types {

class StringTest : public oatpp::test::UnitTest {
public:
  StringTest() : UnitTest("TEST[mariadb::types::StringTest]") {}
  void onRun() override;
};

}}}}

#endif // oatpp_test_mariadb_types_StringTest_hpp
