#ifndef oatpp_mariadb_test_types_FlagTest_hpp
#define oatpp_mariadb_test_types_FlagTest_hpp

#include "oatpp-test/UnitTest.hpp"
#include "oatpp-mariadb/types/Flag.hpp"

namespace oatpp { namespace test { namespace mariadb { namespace types {

class FlagTest : public oatpp::test::UnitTest {
public:
    FlagTest() : UnitTest("TEST[oatpp-mariadb::types::FlagTest]") {}
    void onRun() override;

private:
    void testFlag8();
    void testFlag16();
    void testFlag32();
    void testFlag64();
    void testInvalidValues();
};

}}}} // namespace oatpp::test::mariadb::types

#endif // oatpp_mariadb_test_types_FlagTest_hpp 