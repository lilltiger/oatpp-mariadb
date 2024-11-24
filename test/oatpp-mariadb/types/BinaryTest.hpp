#ifndef oatpp_test_mariadb_types_BinaryTest_hpp
#define oatpp_test_mariadb_types_BinaryTest_hpp

#include "oatpp-test/UnitTest.hpp"

namespace oatpp { namespace test { namespace mariadb { namespace types {

class BinaryTest : public oatpp::test::UnitTest {
public:
  BinaryTest() : UnitTest("TEST[mariadb::types::BinaryTest]") {}
  void onRun() override;
};

}}}}

#endif // oatpp_test_mariadb_types_BinaryTest_hpp
