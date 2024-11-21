#include "NumericTest.hpp"
#include "../utils/EnvLoader.hpp"

#include "oatpp-mariadb/orm.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/Types.hpp"

namespace oatpp { namespace test { namespace mariadb { namespace types {

namespace {

const char* const TAG = "TEST[mariadb::types::NumericTest]";

#include OATPP_CODEGEN_BEGIN(DTO)

class NumsRow : public oatpp::DTO {

  DTO_INIT(NumsRow, DTO);

  DTO_FIELD(Int64, f_number);
  DTO_FIELD(Float64, f_decimal);
  DTO_FIELD(UInt8, f_number_unchar);
  DTO_FIELD(String, f_date);
  DTO_FIELD(String, f_datetime);
  DTO_FIELD(String, f_string);

};

#include OATPP_CODEGEN_END(DTO)

#include OATPP_CODEGEN_BEGIN(DbClient)

class MyClient : public oatpp::orm::DbClient {
public:

  MyClient(const std::shared_ptr<oatpp::orm::Executor>& executor)
    : oatpp::orm::DbClient(executor)
  {
    // create tables
  }

  QUERY(createTable,
        "CREATE TABLE IF NOT EXISTS `test_numerics` ("
        "`f_number` INTEGER,"
        "`f_decimal` DOUBLE PRECISION,"
        "`f_number_unchar` INTEGER,"
        "`f_date` DATE,"
        "`f_datetime` DATETIME,"
        "`f_string` VARCHAR(255)"
        ") ENGINE=InnoDB;")

  QUERY(insertNumValues,
        "INSERT INTO test_numerics "
        "(f_number, f_decimal, f_number_unchar, f_date, f_datetime, f_string) "
        "VALUES "
        "(:row.f_number, :row.f_decimal, :row.f_number_unchar, :row.f_date, :row.f_datetime, :row.f_string);",
        PARAM(oatpp::Object<NumsRow>, row))

  QUERY(deleteAllNums,
        "DELETE FROM test_numerics;")

  QUERY(selectAllNums, "SELECT * FROM test_numerics;")

};

#include OATPP_CODEGEN_END(DbClient)

}

void NumericTest::onRun() {
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
    auto connection = connectionProvider->get();
    if (!connection) {
      OATPP_LOGE(TAG, "Failed to establish database connection");
      throw std::runtime_error("Database connection failed");
    }
    OATPP_LOGD(TAG, "Successfully connected to database");
    
    auto executor = std::make_shared<oatpp::mariadb::Executor>(connectionProvider);
    auto client = MyClient(executor);

    // Create the test_numerics table
    {
      auto res = client.createTable();
      if (!res->isSuccess()) {
        OATPP_LOGE(TAG, "Failed to create table: %s", res->getErrorMessage());
        throw std::runtime_error("Failed to create table");
      }
      OATPP_LOGD(TAG, "Successfully created test_numerics table");
    }

  {
    auto res = client.deleteAllNums();
    if (res->isSuccess()) {
      OATPP_LOGD(TAG, "OK, knownCount=%d, hasMore=%d", res->getKnownCount(), res->hasMoreToFetch());
    } else {
      auto message = res->getErrorMessage();
      OATPP_LOGD(TAG, "Error, message=%s", message);
    }

    OATPP_ASSERT(res->isSuccess());
  }

  {
    auto connection = client.getConnection();
    {
      auto row = NumsRow::createShared();
      row->f_number = nullptr;
      row->f_decimal = nullptr;
      row->f_number_unchar = nullptr;
      row->f_date = nullptr;
      row->f_datetime = nullptr;
      row->f_string = nullptr;
      client.insertNumValues(row, connection);
    }

    {
      auto row = NumsRow::createShared();
      row->f_number = 10;
      row->f_decimal = 10;
      row->f_number_unchar = 1;
      row->f_date = "2020-09-04";
      row->f_datetime = "2020-09-04 00:00:00";
      row->f_string = "bar";
      client.insertNumValues(row, connection);
    }

    OATPP_LOGD(TAG, "Insert 2 rows successfully");
  }

  {
    auto res = client.selectAllNums();
    if(res->isSuccess()) {
      OATPP_LOGD(TAG, "OK, knownCount=%d, hasMore=%d", res->getKnownCount(), res->hasMoreToFetch());
    } else {
      auto message = res->getErrorMessage();
      OATPP_LOGD(TAG, "Error, message=%s", message);
    }

    auto dataset = res->template fetch< oatpp::Vector< oatpp::Object<NumsRow> > >();

    oatpp::parser::json::mapping::ObjectMapper om;
    om.getSerializer()->getConfig()->useBeautifier = true;
    om.getSerializer()->getConfig()->enabledInterpretations = {"mariadb"};
    om.getSerializer()->getConfig()->escapeFlags = 0;  // Don't escape non-ASCII characters

    auto str = om.writeToString(dataset);

    OATPP_LOGD(TAG, "Query result:\n%s", str->c_str());  

    OATPP_ASSERT(dataset->size() == 2);

    {
      auto row = dataset[0];
      OATPP_ASSERT(row->f_number == nullptr);
      OATPP_ASSERT(row->f_decimal == nullptr);
      OATPP_ASSERT(row->f_number_unchar == nullptr);
      OATPP_ASSERT(row->f_date == nullptr);
      OATPP_ASSERT(row->f_datetime == nullptr);
      OATPP_ASSERT(row->f_string == nullptr);
    }

    {
      auto row = dataset[1];
      OATPP_ASSERT(row->f_number == 10);
      OATPP_ASSERT(row->f_decimal == 10);
      OATPP_ASSERT(row->f_number_unchar == 1);
      OATPP_ASSERT(row->f_date == "2020-09-04");
      OATPP_ASSERT(row->f_datetime == "2020-09-04 00:00:00");
      OATPP_ASSERT(row->f_string == "bar");
    }
  }

  } catch (const std::exception& e) {
    OATPP_LOGE(TAG, "An error occurred: %s", e.what());
  }

}

}}}}
