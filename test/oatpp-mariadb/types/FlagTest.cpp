#include "FlagTest.hpp"
#include "oatpp-mariadb/orm.hpp"
#include "oatpp-mariadb/types/Flag.hpp"
#include "oatpp-mariadb/mapping/type/FlagMapping.hpp"
#include "../utils/EnvLoader.hpp"

namespace oatpp { namespace test { namespace mariadb { namespace types {

namespace {

const char* const TAG = "TEST[mariadb::types::FlagTest]";

#include OATPP_CODEGEN_BEGIN(DTO)

class FlagRow : public oatpp::DTO {
  DTO_INIT(FlagRow, DTO)
  DTO_FIELD(oatpp::mariadb::types::Flag<64>, flag_value);
};

#include OATPP_CODEGEN_END(DTO)

#include OATPP_CODEGEN_BEGIN(DbClient)

class MyClient : public oatpp::orm::DbClient {
public:
  explicit MyClient(const std::shared_ptr<oatpp::orm::Executor>& executor)
    : oatpp::orm::DbClient(executor)
  {
    // Register Flag type mapping
    oatpp::mariadb::types::Flag<64>::registerFlag("READ", 1ULL);
    oatpp::mariadb::types::Flag<64>::registerFlag("WRITE", 2ULL);
    oatpp::mariadb::types::Flag<64>::registerFlag("EXECUTE", 4ULL);
    oatpp::mariadb::types::Flag<64>::registerFlag("ALL", 7ULL);
  }

  QUERY(createTable,
        "CREATE TABLE IF NOT EXISTS test_flag ("
        "id INTEGER PRIMARY KEY AUTO_INCREMENT,"
        "flag_value BIGINT UNSIGNED"
        ");")

  QUERY(dropTable,
        "DROP TABLE IF EXISTS test_flag;")

  QUERY(insertFlag,
        "INSERT INTO test_flag (flag_value) VALUES (:row.flag_value);",
        PARAM(oatpp::Object<FlagRow>, row))

  QUERY(selectFlag,
        "SELECT flag_value FROM test_flag LIMIT 1;")
};

#include OATPP_CODEGEN_END(DbClient)

}

void FlagTest::onRun() {
  OATPP_LOGI(TAG, "Testing Flag type...");

  auto env = utils::EnvLoader();
  
  auto options = oatpp::mariadb::ConnectionOptions();
  options.host = env.get("MARIADB_HOST", "127.0.0.1");
  options.port = env.getInt("MARIADB_PORT", 3306);
  options.username = env.get("MARIADB_USER", "root");
  options.password = env.get("MARIADB_PASSWORD", "root");
  options.database = env.get("MARIADB_DATABASE", "test");

  try {
    auto connectionProvider = std::make_shared<oatpp::mariadb::ConnectionProvider>(options);
    auto executor = std::make_shared<oatpp::mariadb::Executor>(connectionProvider);
    auto client = MyClient(executor);

    // Drop and recreate table
    client.dropTable();
    client.createTable();

    // Create test data
    auto row = FlagRow::createShared();
    row->flag_value = oatpp::mariadb::types::Flag<64>(0ULL);  // Initialize with 0
    row->flag_value.setFlag("ALL");  // This should set bits 0, 1, and 2 (value 7)

    // Insert test data
    auto result = client.insertFlag(row);
    OATPP_ASSERT(result->isSuccess());

    // Select and verify
    auto selectResult = client.selectFlag();
    OATPP_ASSERT(selectResult->isSuccess());

    auto value = selectResult->fetch<oatpp::Object<FlagRow>>();
    OATPP_ASSERT(value);
    OATPP_ASSERT(value->flag_value.getValue(0ULL) == 7ULL);
    OATPP_ASSERT(value->flag_value.hasFlag("READ"));
    OATPP_ASSERT(value->flag_value.hasFlag("WRITE"));
    OATPP_ASSERT(value->flag_value.hasFlag("EXECUTE"));
    OATPP_ASSERT(value->flag_value.hasFlag("ALL"));

  } catch (const std::exception& e) {
    OATPP_LOGE(TAG, "Error: %s", e.what());
    throw;
  }
}

}}}}

#include OATPP_CODEGEN_END(DTO) 