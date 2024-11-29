#include "TypeMappingTest.hpp"
#include "../utils/EnvLoader.hpp"

#include "oatpp-mariadb/orm.hpp"
#include "oatpp-mariadb/Types.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"

namespace oatpp { namespace test { namespace mariadb { namespace types {

namespace {

const char* const TAG = "TEST[mariadb::types::TypeMappingTest]";

#include OATPP_CODEGEN_BEGIN(DTO)

class TypeMappingRow : public oatpp::DTO {
  
  DTO_INIT(TypeMappingRow, DTO);

  DTO_FIELD(String, name);           // VARCHAR(255)
  DTO_FIELD(Float64, amount);        // DECIMAL(10,2)
  DTO_FIELD(Int32, age);            // INT
  DTO_FIELD(Int64, big_num);        // BIGINT
  DTO_FIELD(Boolean, is_active);     // BOOLEAN
  DTO_FIELD(String, description);    // TEXT
  DTO_FIELD(String, created_at);     // DATETIME
  DTO_FIELD(String, birth_date);     // DATE
  DTO_FIELD(String, work_hours);     // TIME
  DTO_FIELD(String, data);           // BLOB

};

#include OATPP_CODEGEN_END(DTO)

#include OATPP_CODEGEN_BEGIN(DbClient)

class TestClient : public oatpp::orm::DbClient {
public:
  TestClient(const std::shared_ptr<orm::Executor>& executor)
    : DbClient(executor)
  {}

  QUERY(createTable,
        "CREATE TABLE IF NOT EXISTS test_types ("
        "id INTEGER PRIMARY KEY AUTO_INCREMENT,"
        "name VARCHAR(255),"
        "amount DECIMAL(10,2),"
        "age INT,"
        "big_num BIGINT,"
        "is_active BOOLEAN,"
        "description TEXT,"
        "created_at DATETIME,"
        "birth_date DATE,"
        "work_hours TIME,"
        "data BLOB"
        ") ENGINE=InnoDB;")

  QUERY(insertRow,
        "INSERT INTO test_types ("
        "name, amount, age, big_num, is_active, description, created_at, birth_date, work_hours, data"
        ") VALUES ("
        ":row.name, :row.amount, :row.age, :row.big_num, :row.is_active, :row.description, "
        ":row.created_at, :row.birth_date, :row.work_hours, :row.data"
        ");",
        PARAM(oatpp::Object<TypeMappingRow>, row))

  QUERY(insertWithExplicitTypes,
        "INSERT INTO test_types ("
        "name, amount, age, big_num, is_active, description, created_at, birth_date, work_hours, data"
        ") VALUES ("
        ":name, :amount, :age, :big_num, :is_active, :description, :created_at, :birth_date, :work_hours, :data"
        ");",
        PARAM_VARCHAR(name, 255),
        PARAM_DECIMAL(amount, 10, 2),
        PARAM_INT(age),
        PARAM_BIGINT(big_num),
        PARAM_BOOL(is_active),
        PARAM_TEXT(description),
        PARAM_DATETIME(created_at),
        PARAM_DATE(birth_date),
        PARAM_TIME(work_hours),
        PARAM_BLOB(data))
};

#include OATPP_CODEGEN_END(DbClient)

}

void TypeMappingTest::onRun() {
  OATPP_LOGI(TAG, "Testing explicit type mappings...");

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

  try {
    auto connectionProvider = std::make_shared<oatpp::mariadb::ConnectionProvider>(options);
    auto dbConnection = connectionProvider->get();
    if (!dbConnection) {
      OATPP_LOGE(TAG, "Failed to establish database connection");
      throw std::runtime_error("Database connection failed");
    }
    OATPP_LOGD(TAG, "Successfully connected to database");
    
    auto executor = std::make_shared<oatpp::mariadb::Executor>(connectionProvider);
    auto client = TestClient(executor);

    // Create test table
    {
      auto res = client.createTable();
      if (!res->isSuccess()) {
        OATPP_LOGE(TAG, "Failed to create table: %s", res->getErrorMessage()->c_str());
        throw std::runtime_error("Failed to create table");
      }
      OATPP_LOGD(TAG, "Successfully created test_types table");
    }

    // Test inserting with DTO
    auto row = TypeMappingRow::createShared();
    row->name = "John Doe";
    row->amount = 123.45;
    row->age = 30;
    row->big_num = 1234567890L;
    row->is_active = true;
    row->description = "Test description";
    row->created_at = "2024-01-01 12:00:00";
    row->birth_date = "1994-01-01";
    row->work_hours = "08:00:00";
    row->data = "binary data";

    auto result1 = client.insertRow(row);
    OATPP_ASSERT(result1->isSuccess());
    OATPP_LOGI(TAG, "Insert with DTO successful");

    // Test inserting with explicit type mappings
    auto result2 = client.insertWithExplicitTypes(
      "Jane Doe",           // VARCHAR(255)
      456.78,              // DECIMAL(10,2)
      25,                  // INT
      9876543210L,        // BIGINT
      true,               // BOOLEAN
      "Another test",     // TEXT
      "2024-01-02 15:30:00", // DATETIME
      "1995-02-02",       // DATE
      "09:30:00",         // TIME
      "more binary data"  // BLOB
    );

    OATPP_ASSERT(result2->isSuccess());
    OATPP_LOGI(TAG, "Insert with explicit type mappings successful");

  } catch (const std::exception& e) {
    OATPP_LOGE(TAG, "Error: %s", e.what());
    throw;
  }
}

}}}} 