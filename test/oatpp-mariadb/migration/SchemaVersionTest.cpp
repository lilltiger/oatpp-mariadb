#include "SchemaVersionTest.hpp"
#include "../utils/EnvLoader.hpp"
#include "oatpp-mariadb/orm.hpp"
#include "oatpp-mariadb/Connection.hpp"
#include "oatpp-mariadb/ConnectionProvider.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/utils/ConversionUtils.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp-mariadb/mapping/JsonHelper.hpp"
#include <sstream>
#include "oatpp/core/Types.hpp"
#include "oatpp/core/data/mapping/type/Vector.hpp"
#include "oatpp/core/data/mapping/type/Object.hpp"

namespace oatpp { namespace test { namespace mariadb { namespace migration {

namespace {

#include OATPP_CODEGEN_BEGIN(DTO)

class VersionDTO : public oatpp::DTO {
  DTO_INIT(VersionDTO, DTO);
  DTO_FIELD(oatpp::Int64, current_version);
};

class DescribeRow : public oatpp::DTO {
  DTO_INIT(DescribeRow, DTO);
  DTO_FIELD(String, Field);
  DTO_FIELD(String, Type);
  DTO_FIELD(String, Null);
  DTO_FIELD(String, Key);
  DTO_FIELD(String, Default);
  DTO_FIELD(String, Extra);
};

class SchemaVersionRow : public oatpp::DTO {
  DTO_INIT(SchemaVersionRow, DTO);
  
  DTO_FIELD(oatpp::Int64, version);           // BIGINT for schema version
  DTO_FIELD(String, name);                    // VARCHAR for migration name
  DTO_FIELD(String, script);                  // TEXT for migration script
  DTO_FIELD(String, applied_at);              // DATETIME for when migration was applied
};

#include OATPP_CODEGEN_END(DTO)

#include OATPP_CODEGEN_BEGIN(DbClient)

class MyClient : public oatpp::orm::DbClient {
public:
  explicit MyClient(const std::shared_ptr<oatpp::orm::Executor>& executor)
    : oatpp::orm::DbClient(executor)
  {}

  QUERY(createSchemaVersionTable,
        "CREATE TABLE IF NOT EXISTS `schema_version` ("
        "  `version` BIGINT NOT NULL,"
        "  `name` VARCHAR(255) NOT NULL,"
        "  `script` TEXT NOT NULL,"
        "  `applied_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
        "  PRIMARY KEY (version)"
        ") ENGINE=InnoDB;")

  QUERY(insertVersion,
        "INSERT INTO schema_version "
        "(version, name, script, applied_at) "
        "VALUES "
        "(:row.version, :row.name, :row.script, CURRENT_TIMESTAMP);",
        PARAM(oatpp::Object<SchemaVersionRow>, row))

  QUERY(getVersion,
        "SELECT MAX(version) as current_version FROM schema_version;")

  QUERY(getAllVersions,
        "SELECT * FROM schema_version "
        "ORDER BY version ASC;")
};

#include OATPP_CODEGEN_END(DbClient)

}

void SchemaVersionTest::onRun() {
  OATPP_LOGI(TAG, "Running schema version tests...");

  auto env = utils::EnvLoader();
  
  auto options = oatpp::mariadb::ConnectionOptions();
  options.host = env.get("MARIADB_HOST", "127.0.0.1");
  options.port = env.getInt("MARIADB_PORT", 3306);
  options.username = env.get("MARIADB_USER", "root");
  options.password = env.get("MARIADB_PASSWORD", "root");
  options.database = env.get("MARIADB_DATABASE", "test");

  auto connectionProvider = std::make_shared<oatpp::mariadb::ConnectionProvider>(options);
  auto executor = std::make_shared<oatpp::mariadb::Executor>(connectionProvider);
  auto client = MyClient(executor);

  {
    OATPP_LOGI(TAG, "Test schema versioning with DTO...");
    ConnectionGuard conn(executor);

    // Clean up existing schema_version table
    auto cleanupResult = executor->executeRaw("DROP TABLE IF EXISTS schema_version, test_table", conn.get());
    OATPP_ASSERT(cleanupResult->isSuccess());

    auto result = client.createSchemaVersionTable();
    OATPP_ASSERT(result->isSuccess());

    // Test initial version (nullptr case)
    auto versionResult = client.getVersion();
    OATPP_ASSERT(versionResult->isSuccess());
    auto vectorType = oatpp::Vector<oatpp::Object<SchemaVersionRow>>::Class::getType();
    auto rowsAny = versionResult->fetch(vectorType, 1);
    auto rows = rowsAny.cast<oatpp::Vector<oatpp::Object<SchemaVersionRow>>>();

    oatpp::Int64 currentVersion = nullptr;
    if (rows && !rows->empty()) {
      auto row = rows->at(0);
      if (row) {
        currentVersion = row->version;
      }
    }
    OATPP_ASSERT(currentVersion == nullptr);

    // Test migration to version 1
    auto v1Row = SchemaVersionRow::createShared();
    v1Row->version = static_cast<int64_t>(1);
    v1Row->name = "create_test_table";
    v1Row->script = "CREATE TABLE IF NOT EXISTS test_table ("
                    "  id INT NOT NULL AUTO_INCREMENT,"
                    "  name VARCHAR(255) DEFAULT NULL,"
                    "  PRIMARY KEY (id)"
                    ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci";
    
    result = client.insertVersion(v1Row);
    OATPP_ASSERT(result->isSuccess());

    // Execute the migration script
    result = executor->executeRaw(v1Row->script, conn.get());
    OATPP_ASSERT(result->isSuccess());

    // Debug: Show table structure
    auto descResult = executor->executeRaw("DESCRIBE schema_version", conn.get());
    if (descResult && descResult->isSuccess()) {
      OATPP_LOGD(TAG, "Table structure for schema_version:");
      
      auto dataset = descResult->fetch<oatpp::Vector<oatpp::Object<DescribeRow>>>(1);
      if (dataset && !dataset->empty()) {
        for (const auto& row : *dataset) {
          std::stringstream ss;
          ss << "Field: " << row->Field->c_str() << "\t";
          ss << "Type: " << row->Type->c_str() << "\t";
          ss << "Null: " << (row->Null ? row->Null->c_str() : "NULL") << "\t";
          ss << "Key: " << (row->Key ? row->Key->c_str() : "NULL") << "\t";
          ss << "Default: " << (row->Default ? row->Default->c_str() : "NULL") << "\t";
          ss << "Extra: " << (row->Extra ? row->Extra->c_str() : "NULL");
          OATPP_LOGD(TAG, "%s", ss.str().c_str());
        }
      }
    }

    // Debug: Show actual data
    auto dataResult = executor->executeRaw("SELECT * FROM schema_version", conn.get());
    if (dataResult && dataResult->isSuccess()) {
      auto data = std::static_pointer_cast<oatpp::mariadb::QueryResult>(dataResult);
      OATPP_LOGD(TAG, "Current data in schema_version:");
      auto vectorType = oatpp::Vector<oatpp::Object<SchemaVersionRow>>::Class::getType();
      auto rowsAny = data->fetch(vectorType, 1);
      auto rows = rowsAny.cast<oatpp::Vector<oatpp::Object<SchemaVersionRow>>>();
      if (rows && !rows->empty()) {
        for (const auto& row : *rows) {
          OATPP_LOGD(TAG, "Version: %s, Name: '%s', Script: '%s', Applied At: '%s'",
                     versionToString(row->version)->c_str(),
                     row->name->c_str(),
                     row->script->c_str(),
                     row->applied_at->c_str());
        }
      } else {
        OATPP_LOGD(TAG, "No rows found in schema_version");
      }
    } else {
      OATPP_LOGE(TAG, "Failed to fetch schema_version data");
    }

    // Now try to get version
    auto rawResult = executor->executeRaw("SELECT MAX(version) as current_version FROM schema_version", conn.get());
    if (rawResult && rawResult->isSuccess()) {
      auto result = std::static_pointer_cast<oatpp::mariadb::QueryResult>(rawResult);
      auto vectorType = oatpp::Vector<oatpp::Object<VersionDTO>>::Class::getType();
      auto rowsAny = result->fetch(vectorType, 1);
      auto rows = rowsAny.cast<oatpp::Vector<oatpp::Object<VersionDTO>>>();
      if (rows && !rows->empty()) {
        currentVersion = rows->front()->current_version;
        OATPP_LOGD(TAG, "Retrieved version: %s", versionToString(currentVersion)->c_str());
      } else {
        currentVersion = nullptr; // No version found
        OATPP_LOGD(TAG, "No version found, using default: 0");
      }
    }
    
    v_int64 expectedVersion = 1;
    OATPP_LOGD(TAG, "Comparing versions: current=%d, expected=%d", 
               (v_int64)currentVersion, 
               expectedVersion);
               
    OATPP_ASSERT((v_int64)currentVersion == expectedVersion);
  }

  {
    OATPP_LOGI(TAG, "Test error handling...");
    ConnectionGuard conn(executor);

    // Test empty script
    auto emptyRow = SchemaVersionRow::createShared();
    emptyRow->version = static_cast<int64_t>(2);
    emptyRow->name = "empty_script";
    emptyRow->script = "";
    
    bool emptyScriptCaught = false;
    try {
      auto scriptError = validateScript(emptyRow->script);
      OATPP_ASSERT(!scriptError.empty());
      if (!scriptError.empty()) {
        throw MigrationError(scriptError);
      }
      executor->executeRaw(emptyRow->script, conn.get());
    } catch (const MigrationError& e) {
      emptyScriptCaught = true;
      OATPP_LOGD(TAG, "Empty script error caught as expected: %s", e.what());
    }
    OATPP_ASSERT(emptyScriptCaught);

    // Test invalid version (negative)
    auto invalidRow = SchemaVersionRow::createShared();
    invalidRow->version = static_cast<int64_t>(-1);
    invalidRow->name = "invalid_version";
    invalidRow->script = "SELECT 1";
    
    bool invalidVersionCaught = false;
    try {
      auto versionError = validateVersion(invalidRow->version);
      OATPP_ASSERT(!versionError.empty());
      if (!versionError.empty()) {
        throw MigrationError(versionError);
      }
      client.insertVersion(invalidRow);
    } catch (const MigrationError& e) {
      invalidVersionCaught = true;
      OATPP_LOGD(TAG, "Invalid version error caught as expected: %s", e.what());
    }
    OATPP_ASSERT(invalidVersionCaught);

    // Test max version overflow
    auto overflowRow = SchemaVersionRow::createShared();
    overflowRow->version = MAX_VERSION;
    overflowRow->name = "overflow_version";
    overflowRow->script = "SELECT 1";
    
    bool overflowCaught = false;
    try {
      auto nextVersion = oatpp::Int64(overflowRow->version + 1);
      auto versionError = validateVersion(nextVersion);
      OATPP_ASSERT(!versionError.empty());
      if (!versionError.empty()) {
        throw MigrationError(versionError);
      }
      client.insertVersion(overflowRow);
    } catch (const MigrationError& e) {
      overflowCaught = true;
      OATPP_LOGD(TAG, "Version overflow error caught as expected: %s", e.what());
    }
    OATPP_ASSERT(overflowCaught);

    // Test invalid SQL
    auto invalidSqlRow = SchemaVersionRow::createShared();
    invalidSqlRow->version = static_cast<int64_t>(2);
    invalidSqlRow->name = "invalid_sql";
    invalidSqlRow->script = "INVALID SQL STATEMENT";
    
    bool invalidSqlCaught = false;
    try {
      auto scriptError = validateScript(invalidSqlRow->script);
      OATPP_ASSERT(scriptError.empty());  // Script content is valid, but SQL is invalid
      executor->executeRaw(invalidSqlRow->script, conn.get());
    } catch (const std::exception& e) {
      invalidSqlCaught = true;
      OATPP_LOGD(TAG, "Invalid SQL error caught as expected: %s", e.what());
    }
    OATPP_ASSERT(invalidSqlCaught);
  }

  {
    OATPP_LOGI(TAG, "Test version ordering with gaps...");
    ConnectionGuard conn(executor);

    // Clean up existing schema_version table before testing
    auto cleanupResult = executor->executeRaw("DROP TABLE IF EXISTS schema_version", conn.get());
    OATPP_ASSERT(cleanupResult->isSuccess());
    
    // Recreate the table
    auto createResult = client.createSchemaVersionTable();
    OATPP_ASSERT(createResult->isSuccess());

    // Insert versions with gaps
    std::vector<int64_t> versions = {5, 10, 15, 100};  
    for(auto version : versions) {
      auto row = SchemaVersionRow::createShared();
      row->version = version;
      row->name = "version_" + std::to_string(version);
      row->script = "SELECT " + std::to_string(version);
      
      OATPP_LOGD(TAG, "Inserting version %d...", (v_int64)version);
      auto result = client.insertVersion(row);
      OATPP_ASSERT(result->isSuccess());
    }

    // Debug: Verify data directly
    auto debugResult = executor->executeRaw("SELECT version, name, script FROM schema_version ORDER BY version ASC", conn.get());
    OATPP_ASSERT(debugResult->isSuccess());
    auto debugRows = debugResult->fetch<oatpp::Vector<oatpp::Object<SchemaVersionRow>>>();
    OATPP_LOGD(TAG, "Direct query found %d rows", debugRows->size());
    for(const auto& row : *debugRows) {
      OATPP_LOGD(TAG, "Debug - Version: %s, Name: %s", 
                 versionToString(row->version)->c_str(),
                 row->name->c_str());
    }

    // Verify versions are stored in order
    auto allVersions = client.getAllVersions();
    OATPP_ASSERT(allVersions->isSuccess());
    auto vectorType = oatpp::Vector<oatpp::Object<SchemaVersionRow>>::Class::getType();
    auto versionRowsAny = allVersions->fetch(vectorType, -1);  // Use -1 to get all rows
    auto versionRows = versionRowsAny.cast<oatpp::Vector<oatpp::Object<SchemaVersionRow>>>();
    
    // Log versions for debugging
    OATPP_LOGD(TAG, "Verifying version order - found %d versions", versionRows->size());
    v_int64 previousVersion = 0;
    for(const auto& row : *versionRows) {
      if (row && row->version) {
        OATPP_LOGD(TAG, "Version: %s, Name: %s", 
                   versionToString(row->version)->c_str(),
                   row->name->c_str());
        OATPP_ASSERT(*row->version > previousVersion);  // Verify ascending order
        previousVersion = *row->version;
      }
    }

    // Verify we have all expected versions
    OATPP_ASSERT(versionRows->size() == versions.size());
  }

  OATPP_LOGI(TAG, "Schema version tests completed successfully");
}

}}}}
