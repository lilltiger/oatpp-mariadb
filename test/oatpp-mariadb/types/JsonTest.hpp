#ifndef oatpp_test_mariadb_types_JsonTest_hpp
#define oatpp_test_mariadb_types_JsonTest_hpp

#include "oatpp-test/UnitTest.hpp"

namespace oatpp { namespace test { namespace mariadb { namespace types {

class JsonTest : public oatpp::test::UnitTest {
public:
  JsonTest() : UnitTest("TEST[mariadb::types::JsonTest]") {}
  void onRun() override;
};

}}}}

#endif // oatpp_test_mariadb_types_JsonTest_hpp
