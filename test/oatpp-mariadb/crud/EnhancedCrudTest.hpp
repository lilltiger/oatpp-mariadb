#ifndef EnhancedCrudTest_hpp
#define EnhancedCrudTest_hpp

#include "oatpp-test/UnitTest.hpp"

namespace oatpp { namespace test { namespace mariadb { namespace crud {

class EnhancedCrudTest : public oatpp::test::UnitTest {
public:
  EnhancedCrudTest() : UnitTest("TEST[mariadb::crud::EnhancedCrudTest]") {}
  void onRun() override;
};

}}}}

#endif // EnhancedCrudTest_hpp
