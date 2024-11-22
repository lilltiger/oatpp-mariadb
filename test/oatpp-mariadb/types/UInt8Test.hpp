#ifndef oatpp_test_mariadb_types_UInt8Test_hpp
#define oatpp_test_mariadb_types_UInt8Test_hpp

#include "oatpp-test/UnitTest.hpp"

namespace oatpp { namespace test { namespace mariadb { namespace types {

class UInt8Test : public UnitTest {
public:
  UInt8Test() : UnitTest("TEST[mariadb::types::UInt8Test]") {}
  void onRun() override;
};

}}}}

#endif // oatpp_test_mariadb_types_UInt8Test_hpp
