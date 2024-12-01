#ifndef oatpp_test_mariadb_types_CustomTypeTest_hpp
#define oatpp_test_mariadb_types_CustomTypeTest_hpp

#include "oatpp-test/UnitTest.hpp"
#include "oatpp-mariadb/orm.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/Types.hpp"

namespace oatpp { namespace test { namespace mariadb { namespace types {

#include OATPP_CODEGEN_BEGIN(DTO)

class CustomTypeRow : public oatpp::DTO {
  DTO_INIT(CustomTypeRow, DTO)
  DTO_FIELD(String, data);
};

#include OATPP_CODEGEN_END(DTO)

#include OATPP_CODEGEN_BEGIN(DbClient)

class MyClient : public oatpp::orm::DbClient {
public:
  explicit MyClient(const std::shared_ptr<oatpp::orm::Executor>& executor)
    : oatpp::orm::DbClient(executor)
  {}

  QUERY(createTable,
        "CREATE TABLE IF NOT EXISTS custom_type_test ("
        "  id INTEGER PRIMARY KEY AUTO_INCREMENT,"
        "  data JSON"
        ") ENGINE=InnoDB;")

  QUERY(deleteAll,
        "DELETE FROM custom_type_test;")

  QUERY(insertValue,
        "INSERT INTO custom_type_test (data) VALUES (:row.data);",
        PARAM(oatpp::Object<CustomTypeRow>, row))

  QUERY(updateValue,
        "UPDATE custom_type_test "
        "SET data = :row.data "
        "WHERE JSON_EXTRACT(data, '$.customer_id') = JSON_EXTRACT(:row.data, '$.customer_id');",
        PARAM(oatpp::Object<CustomTypeRow>, row))

  QUERY(selectAll,
        "SELECT * FROM custom_type_test ORDER BY id;")

  QUERY(dropTable,
        "DROP TABLE IF EXISTS custom_type_test;")
};

#include OATPP_CODEGEN_END(DbClient)

class CustomTypeTest : public UnitTest {
public:
  CustomTypeTest() : UnitTest("TEST[mariadb::types::CustomTypeTest]") {}
  void onRun() override;
};

}}}}

#endif // oatpp_test_mariadb_types_CustomTypeTest_hpp 