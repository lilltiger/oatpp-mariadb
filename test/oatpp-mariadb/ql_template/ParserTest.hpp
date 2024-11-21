#ifndef oatpp_test_mariadb_ql_template_ParserTest_hpp
#define oatpp_test_mariadb_ql_template_ParserTest_hpp

#include "oatpp-test/UnitTest.hpp"

namespace oatpp { namespace test { namespace mariadb { namespace ql_template {

class ParserTest : public UnitTest {
public:
  ParserTest() : UnitTest("TEST[mariadb::ql_template::ParserTest]") {}
  void onRun() override;
};

}}}}

#endif // oatpp_test_mariadb_ql_template_ParserTest_hpp
