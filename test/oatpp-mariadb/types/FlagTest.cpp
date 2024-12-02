#include "FlagTest.hpp"
#include "../utils/EnvLoader.hpp"

#include "oatpp-mariadb/orm.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp-mariadb/types/Flag.hpp"

namespace oatpp { namespace test { namespace mariadb { namespace types {

namespace {

const char* const TAG = "TEST[mariadb::types::FlagTest]";

#include OATPP_CODEGEN_BEGIN(DTO)

class FlagRow : public oatpp::DTO {
    DTO_INIT(FlagRow, DTO);
    DTO_FIELD(Int64, id);
    DTO_FIELD(UInt64, permissions);
};

#include OATPP_CODEGEN_END(DTO)

#include OATPP_CODEGEN_BEGIN(DbClient)

class TestClient : public oatpp::orm::DbClient {
public:
    TestClient(const std::shared_ptr<oatpp::orm::Executor>& executor)
        : oatpp::orm::DbClient(executor)
    {}
    
    QUERY(createTable,
          "CREATE TABLE IF NOT EXISTS `flag_test` ("
          "`id` BIGINT NOT NULL AUTO_INCREMENT,"
          "`permissions` BIGINT UNSIGNED NOT NULL,"
          "PRIMARY KEY (`id`)"
          ") ENGINE=InnoDB;")
    
    QUERY(dropTable,
          "DROP TABLE IF EXISTS `flag_test`;")
    
    QUERY(insertRow,
          "INSERT INTO `flag_test` (`permissions`) VALUES (:row.permissions) RETURNING *;",
          PARAM(oatpp::Object<FlagRow>, row))
    
    QUERY(selectAll,
          "SELECT * FROM `flag_test` ORDER BY `id`;")
    
    QUERY(deleteAll,
          "DELETE FROM `flag_test`;")
};

#include OATPP_CODEGEN_END(DbClient)

}

void FlagTest::onRun() {
    OATPP_LOGD(TAG, "Running Flag Type Tests...");

    auto env = utils::EnvLoader();
    
    auto options = oatpp::mariadb::ConnectionOptions();
    options.host = env.get("MARIADB_HOST", "127.0.0.1");
    options.port = env.getInt("MARIADB_PORT", 3306);
    options.username = env.get("MARIADB_USER", "root");
    options.password = env.get("MARIADB_PASSWORD", "root");
    options.database = env.get("MARIADB_DATABASE", "test");

    auto connectionProvider = std::make_shared<oatpp::mariadb::ConnectionProvider>(options);
    auto executor = std::make_shared<oatpp::mariadb::Executor>(connectionProvider);
    auto client = TestClient(executor);

    // Setup: create fresh table
    client.dropTable();
    client.createTable();

    // Register flag values
    oatpp::mariadb::types::Flag::registerFlag("READ", 1);
    oatpp::mariadb::types::Flag::registerFlag("WRITE", 2);
    oatpp::mariadb::types::Flag::registerFlag("EXECUTE", 4);
    oatpp::mariadb::types::Flag::registerFlag("ADMIN", 8);

    // Test basic flag operations
    {
        OATPP_LOGD(TAG, "Testing basic flag operations...");
        
        auto row = FlagRow::createShared();
        row->permissions = oatpp::UInt64((v_uint64)3); // READ | WRITE

        auto res = client.insertRow(row);
        OATPP_ASSERT(res->isSuccess());
        
        auto insertedRow = res->fetch<oatpp::Object<FlagRow>>();
        OATPP_ASSERT(insertedRow);
        auto flagValue = insertedRow->permissions;
        OATPP_ASSERT((flagValue & 1) == 1); // Has READ
        OATPP_ASSERT((flagValue & 2) == 2); // Has WRITE
        OATPP_ASSERT((flagValue & 4) == 0); // No EXECUTE
        OATPP_ASSERT((flagValue & 8) == 0); // No ADMIN
    }

    // Test all flags
    {
        OATPP_LOGD(TAG, "Testing all flags...");
        
        auto row = FlagRow::createShared();
        row->permissions = oatpp::UInt64((v_uint64)15); // READ | WRITE | EXECUTE | ADMIN

        auto res = client.insertRow(row);
        OATPP_ASSERT(res->isSuccess());
        
        auto insertedRow = res->fetch<oatpp::Object<FlagRow>>();
        OATPP_ASSERT(insertedRow);
        auto flagValue = insertedRow->permissions;
        OATPP_ASSERT((flagValue & 1) == 1); // Has READ
        OATPP_ASSERT((flagValue & 2) == 2); // Has WRITE
        OATPP_ASSERT((flagValue & 4) == 4); // Has EXECUTE
        OATPP_ASSERT((flagValue & 8) == 8); // Has ADMIN
    }

    // Test zero flags
    {
        OATPP_LOGD(TAG, "Testing zero flags...");
        
        auto row = FlagRow::createShared();
        row->permissions = oatpp::UInt64((v_uint64)0);

        auto res = client.insertRow(row);
        OATPP_ASSERT(res->isSuccess());
        
        auto insertedRow = res->fetch<oatpp::Object<FlagRow>>();
        OATPP_ASSERT(insertedRow);
        auto flagValue = insertedRow->permissions;
        OATPP_ASSERT(flagValue == 0);
    }

    // Verify all test cases
    {
        auto res = client.selectAll();
        OATPP_ASSERT(res->isSuccess());
        
        auto dataset = res->fetch<oatpp::Vector<oatpp::Object<FlagRow>>>();
        OATPP_ASSERT(dataset->size() == 3);
        OATPP_LOGD(TAG, "Successfully verified all %d test cases", dataset->size());
    }

    OATPP_LOGD(TAG, "Flag Type Tests finished successfully!");
}

}}}}

void runFlagTests() {
    OATPP_RUN_TEST(oatpp::test::mariadb::types::FlagTest);
} 