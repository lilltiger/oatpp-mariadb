#ifndef oatpp_test_mariadb_types_StatusTest_hpp
#define oatpp_test_mariadb_types_StatusTest_hpp

#include "oatpp-test/UnitTest.hpp"
#include "oatpp-mariadb/types/Status.hpp"
#include "oatpp-mariadb/orm.hpp"
#include "oatpp/core/macro/codegen.hpp"

namespace oatpp { namespace test { namespace mariadb { namespace types {

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * DTO for testing Status type
 */
class StatusRow : public oatpp::DTO {
    DTO_INIT(StatusRow, DTO)
    
    DTO_FIELD(String, status);
    DTO_FIELD(String, description);
};

#include OATPP_CODEGEN_END(DTO)

#include OATPP_CODEGEN_BEGIN(DbClient)

/**
 * Test database client for Status tests
 */
class StatusTestClient : public oatpp::orm::DbClient {
public:
    explicit StatusTestClient(const std::shared_ptr<oatpp::orm::Executor>& executor)
        : oatpp::orm::DbClient(executor)
    {}
    
    QUERY(createTable,
          "CREATE TABLE IF NOT EXISTS status_test ("
          "  id INTEGER PRIMARY KEY AUTO_INCREMENT,"
          "  status ENUM('DRAFT', 'PENDING', 'ACTIVE', 'SUSPENDED', 'CANCELLED', 'COMPLETED') NOT NULL,"
          "  description VARCHAR(255)"
          ") ENGINE=InnoDB;")
    
    QUERY(dropTable,
          "DROP TABLE IF EXISTS status_test;")
    
    QUERY(insertRow,
          "INSERT INTO status_test (status, description) VALUES (:row.status, :row.description);",
          PARAM(oatpp::Object<StatusRow>, row))
    
    QUERY(selectAll,
          "SELECT * FROM status_test ORDER BY id;")
    
    QUERY(deleteAll,
          "DELETE FROM status_test;")
};

#include OATPP_CODEGEN_END(DbClient)

/**
 * Status type test
 */
class StatusTest : public oatpp::test::UnitTest {
private:
    void setupTestStatuses();
    void runDatabaseTests(const std::shared_ptr<oatpp::orm::Executor>& executor);
    
public:
    StatusTest() : UnitTest("TEST[mariadb::types::StatusTest]") {}
    void onRun() override;
};

}}}}

#endif // oatpp_test_mariadb_types_StatusTest_hpp 