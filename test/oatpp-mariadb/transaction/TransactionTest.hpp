#ifndef oatpp_test_mariadb_transaction_TransactionTest_hpp
#define oatpp_test_mariadb_transaction_TransactionTest_hpp

#include "oatpp-test/UnitTest.hpp"

namespace oatpp { namespace test { namespace mariadb { namespace transaction {

class TransactionTest : public oatpp::test::UnitTest {
public:
  TransactionTest() : UnitTest("TEST[mariadb::transaction::TransactionTest]") {}
  void onRun() override;
};

}}}}

#endif // oatpp_test_mariadb_transaction_TransactionTest_hpp
