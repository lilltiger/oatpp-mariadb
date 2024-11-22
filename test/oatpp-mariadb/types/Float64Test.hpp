#ifndef oatpp_test_mariadb_types_Float64Test_hpp
#define oatpp_test_mariadb_types_Float64Test_hpp

#include "oatpp-test/UnitTest.hpp"

namespace oatpp { namespace test { namespace mariadb { namespace types {

class Float64Test : public UnitTest {
public:
  Float64Test() : UnitTest("TEST[mariadb::types::Float64Test]") {}
  void onRun() override;
};

}}}}

#endif // oatpp_test_mariadb_types_Float64Test_hpp
