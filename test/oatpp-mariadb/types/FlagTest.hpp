#ifndef oatpp_test_mariadb_types_FlagTest_hpp
#define oatpp_test_mariadb_types_FlagTest_hpp

#include "oatpp-test/UnitTest.hpp"

namespace oatpp { namespace test { namespace mariadb { namespace types {

class FlagTest : public oatpp::test::UnitTest {
public:
    FlagTest() : UnitTest("TEST[mariadb::types::FlagTest]") {}
    void onRun() override;
};

}}}}

#endif // oatpp_test_mariadb_types_FlagTest_hpp 