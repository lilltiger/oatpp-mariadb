#ifndef SetTest_hpp
#define SetTest_hpp

#include "oatpp-test/UnitTest.hpp"

namespace oatpp { namespace test { namespace mariadb { namespace types {

class SetTest : public oatpp::test::UnitTest {
public:
  SetTest() : UnitTest("TEST[mariadb::types::SetTest]") {}
  void onRun() override;
};

}}}}

#endif // SetTest_hpp
