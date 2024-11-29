#include "QueryResultTest.hpp"
#include "utils/EnvLoader.hpp"

#include "oatpp-mariadb/orm.hpp"
#include "oatpp-mariadb/QueryResult.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/Types.hpp"

namespace oatpp { namespace test { namespace mariadb {

namespace {

const char* const TAG = "TEST[mariadb::QueryResultTest]";

#include OATPP_CODEGEN_BEGIN(DTO)

class TestDto : public oatpp::DTO {
  DTO_INIT(TestDto, DTO);
  DTO_FIELD(Int32, id);
  DTO_FIELD(String, name);
};

#include OATPP_CODEGEN_END(DTO)

#include OATPP_CODEGEN_BEGIN(DbClient)

class TestClient : public oatpp::orm::DbClient {
public:
  TestClient(const std::shared_ptr<oatpp::orm::Executor>& executor)
    : oatpp::orm::DbClient(executor)
  {
  }

  QUERY(createTable,
        "CREATE TABLE IF NOT EXISTS test_query_result ("
        "  id INT PRIMARY KEY,"
        "  name VARCHAR(50)"
        ");")

  QUERY(dropTable, "DROP TABLE IF EXISTS test_query_result;")

  QUERY(insertValues,
        "INSERT INTO test_query_result (id, name) VALUES (1, 'test1'), (2, 'test2');")

  QUERY(selectAll, "SELECT * FROM test_query_result")

};

#include OATPP_CODEGEN_END(DbClient)

}

void QueryResultTest::onRun() {

  OATPP_LOGI(TAG, "Test started");

  // Load environment variables from .env file
  auto env = oatpp::test::mariadb::utils::EnvLoader();
  
  auto options = oatpp::mariadb::ConnectionOptions();
  options.host = env.get("MARIADB_HOST", "127.0.0.1");
  options.port = env.getInt("MARIADB_PORT", 3306);
  options.username = env.get("MARIADB_USER", "root");
  options.password = env.get("MARIADB_PASSWORD", "root");
  options.database = env.get("MARIADB_DATABASE", "test");

  OATPP_LOGD(TAG, "Attempting to connect to database '%s' on '%s:%d' as user '%s'", 
             options.database.getValue("").c_str(), 
             options.host.getValue("").c_str(), 
             options.port,
             options.username.getValue("").c_str());

  auto connectionProvider = std::make_shared<oatpp::mariadb::ConnectionProvider>(options);
  auto executor = std::make_shared<oatpp::mariadb::Executor>(connectionProvider);
  auto client = TestClient(executor);

  // Test hasBeenFetched tracking
  {
    OATPP_LOGI(TAG, "Test hasBeenFetched tracking");

    client.dropTable();
    client.createTable();
    client.insertValues();

    auto dbResult = client.selectAll();
    auto mariaDbResult = std::static_pointer_cast<oatpp::mariadb::QueryResult>(dbResult);
    OATPP_ASSERT(!mariaDbResult->hasBeenFetched());

    auto vectorType = oatpp::Vector<oatpp::Object<TestDto>>::Class::getType();
    auto result = mariaDbResult->fetch(vectorType, 10);
    OATPP_ASSERT(mariaDbResult->hasBeenFetched());
    auto typedResult = result.cast<oatpp::Vector<oatpp::Object<TestDto>>>();
    OATPP_ASSERT(typedResult->size() == 2);

    // Test after second fetch (without caching)
    auto result2 = mariaDbResult->fetch(vectorType, 10);
    OATPP_ASSERT(result2 == nullptr);
  }

  // Test result caching
  {
    OATPP_LOGI(TAG, "Test result caching");

    auto dbResult = client.selectAll();
    auto mariaDbResult = std::static_pointer_cast<oatpp::mariadb::QueryResult>(dbResult);
    mariaDbResult->enableResultCaching(true);
    OATPP_ASSERT(mariaDbResult->isResultCachingEnabled());

    // First fetch
    auto vectorType = oatpp::Vector<oatpp::Object<TestDto>>::Class::getType();
    auto result1 = mariaDbResult->fetch(vectorType, 10);
    auto typedResult1 = result1.cast<oatpp::Vector<oatpp::Object<TestDto>>>();
    OATPP_ASSERT(typedResult1->size() == 2);
    OATPP_ASSERT(typedResult1[0]->id == 1);
    OATPP_ASSERT(typedResult1[0]->name == "test1");

    // Second fetch should return cached results
    auto result2 = mariaDbResult->fetch(vectorType, 10);
    auto typedResult2 = result2.cast<oatpp::Vector<oatpp::Object<TestDto>>>();
    OATPP_ASSERT(typedResult2->size() == 2);
    OATPP_ASSERT(typedResult2[0]->id == 1);
    OATPP_ASSERT(typedResult2[0]->name == "test1");

    // Disable caching
    mariaDbResult->enableResultCaching(false);
    OATPP_ASSERT(!mariaDbResult->isResultCachingEnabled());

    // Third fetch should return null since results were already fetched
    auto result3 = mariaDbResult->fetch(vectorType, 10);
    OATPP_ASSERT(result3 == nullptr);
  }

  // Clean up
  client.dropTable();

  OATPP_LOGI(TAG, "Test finished");

}

}}} 