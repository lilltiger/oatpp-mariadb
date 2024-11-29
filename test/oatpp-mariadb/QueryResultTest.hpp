#ifndef oatpp_test_mariadb_QueryResultTest_hpp
#define oatpp_test_mariadb_QueryResultTest_hpp

#include "oatpp-test/UnitTest.hpp"

namespace oatpp { namespace test { namespace mariadb {

class QueryResultTest : public oatpp::test::UnitTest {
public:
  QueryResultTest() : UnitTest("TEST[mariadb::QueryResultTest]") {}
  ~QueryResultTest() override = default;
  void onRun() override;
};

}}}

#endif // oatpp_test_mariadb_QueryResultTest_hpp 