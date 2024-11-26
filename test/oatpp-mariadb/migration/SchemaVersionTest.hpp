#ifndef oatpp_test_mariadb_migration_SchemaVersionTest_hpp
#define oatpp_test_mariadb_migration_SchemaVersionTest_hpp

#include "oatpp-mariadb/orm.hpp"
#include "oatpp-mariadb/ConnectionProvider.hpp"
#include "oatpp-test/UnitTest.hpp"
#include "oatpp/core/base/Environment.hpp"
#include "oatpp/core/utils/ConversionUtils.hpp"

namespace oatpp { namespace test { namespace mariadb { namespace migration {

class SchemaVersionTest : public oatpp::test::UnitTest {
private:
  static constexpr const char* TAG = "TEST[mariadb::migration::SchemaVersionTest]";
  static constexpr int64_t MIN_VERSION = 0;
  static constexpr int64_t MAX_VERSION = 9223372036854775807LL;  // Max BIGINT
  static constexpr v_int32 MAX_SCRIPT_LENGTH = 1024 * 1024;  // 1MB

  static std::string validateVersion(const oatpp::Int64& version) {
    if (!version) {
      return "Version cannot be null";
    }
    if (*version < MIN_VERSION) {
      return "Version cannot be negative";
    }
    if (*version > MAX_VERSION) {
      return "Version exceeds maximum allowed value";
    }
    return "";
  }

  static bool isValidVersion(const oatpp::Int64& version) {
    return validateVersion(version).empty();
  }

  static std::string validateScript(const oatpp::String& script) {
    if (!script) {
      return "Script cannot be null";
    }
    if (script->empty()) {
      return "Script cannot be empty";
    }
    if (script->length() > MAX_SCRIPT_LENGTH) {
      return "Script exceeds maximum allowed length";
    }
    return "";
  }

  static bool isValidScript(const oatpp::String& script) {
    return validateScript(script).empty();
  }

  static oatpp::String versionToString(const oatpp::Int64& version) {
    if (!version) {
      return "null";
    }
    return oatpp::utils::conversion::int64ToStdStr(*version);
  }

  class UniqueTableName {
  private:
    oatpp::String m_name;
  public:
    UniqueTableName() {
      auto timestamp = oatpp::Int64(oatpp::base::Environment::getMicroTickCount());
      m_name = oatpp::String("test_") + oatpp::utils::conversion::int64ToStdStr(*timestamp);
    }

    operator oatpp::String() const { return m_name; }
    operator const char*() const { return m_name->c_str(); }
    operator std::string() const { return std::string(m_name->c_str()); }
  };

  class MigrationError : public std::runtime_error {
  public:
    MigrationError(const std::string& message) : std::runtime_error(message) {}
  };

  class ConnectionGuard {
  private:
    std::shared_ptr<oatpp::mariadb::Executor> m_executor;
    oatpp::provider::ResourceHandle<oatpp::orm::Connection> m_connection;

  public:
    ConnectionGuard(const std::shared_ptr<oatpp::mariadb::Executor>& executor)
      : m_executor(executor)
      , m_connection(executor->getConnection())
    {
      if (!m_connection) {
        throw std::runtime_error("Failed to get database connection");
      }
    }

    ~ConnectionGuard() {
      if (m_connection) {
        m_executor->closeConnection(m_connection);
      }
    }

    oatpp::provider::ResourceHandle<oatpp::orm::Connection>& get() { return m_connection; }
  };

public:
  SchemaVersionTest() : UnitTest("TEST[mariadb::migration::SchemaVersionTest]") {}
  void onRun() override;
};

}}}}

#endif // oatpp_test_mariadb_migration_SchemaVersionTest_hpp
