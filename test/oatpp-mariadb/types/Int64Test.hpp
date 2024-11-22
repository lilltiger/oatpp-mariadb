#ifndef oatpp_test_mariadb_types_Int64Test_hpp
#define oatpp_test_mariadb_types_Int64Test_hpp

#include "oatpp-test/UnitTest.hpp"

namespace oatpp { namespace test { namespace mariadb { namespace types {

class Int64Test : public UnitTest {
public:
  Int64Test() : UnitTest("TEST[mariadb::types::Int64Test]") {}
  void onRun() override;
};

}}}}

#endif // oatpp_test_mariadb_types_Int64Test_hpp
