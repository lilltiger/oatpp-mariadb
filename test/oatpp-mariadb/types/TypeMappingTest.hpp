#ifndef oatpp_test_mariadb_types_TypeMappingTest_hpp
#define oatpp_test_mariadb_types_TypeMappingTest_hpp

#include "oatpp-test/UnitTest.hpp"
#include "oatpp-mariadb/Types.hpp"

namespace oatpp { namespace test { namespace mariadb { namespace types {

class TypeMappingTest : public UnitTest {
public:
  TypeMappingTest() : UnitTest("TEST[mariadb::types::TypeMappingTest]") {}
  void onRun() override;
};

}}}}

#endif // oatpp_test_mariadb_types_TypeMappingTest_hpp 