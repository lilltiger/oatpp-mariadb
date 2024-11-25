#ifndef oatpp_test_mariadb_types_Int32Test_hpp
#define oatpp_test_mariadb_types_Int32Test_hpp

#include "oatpp-test/UnitTest.hpp"

namespace oatpp { namespace test { namespace mariadb { namespace types {

class Int32Test : public oatpp::test::UnitTest {
public:
  Int32Test() : UnitTest("TEST[mariadb::types::Int32Test]") {}
  void onRun() override;
};

}}}}

#endif // oatpp_test_mariadb_types_Int32Test_hpp
