#include "StatusTest.hpp"
#include "../utils/EnvLoader.hpp"

#include "oatpp-mariadb/orm.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/macro/codegen.hpp"

namespace oatpp { namespace test { namespace mariadb { namespace types {

namespace {
  const char* const TAG = "TEST[mariadb::types::StatusTest]";
}

void StatusTest::onRun() {
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
    auto client = StatusTestClient(executor);

    // Initialize valid statuses and transitions
    {
      // Define valid statuses
      oatpp::mariadb::types::Status::addValidStatus("DRAFT");
      oatpp::mariadb::types::Status::addValidStatus("PENDING");
      oatpp::mariadb::types::Status::addValidStatus("ACTIVE");
      oatpp::mariadb::types::Status::addValidStatus("SUSPENDED");
      oatpp::mariadb::types::Status::addValidStatus("CANCELLED");
      oatpp::mariadb::types::Status::addValidStatus("COMPLETED");
      OATPP_LOGD(TAG, "Initialized valid statuses");
      
      // Define valid transitions
      oatpp::mariadb::types::Status::addTransition("DRAFT", "PENDING");
      oatpp::mariadb::types::Status::addTransition("PENDING", "ACTIVE");
      oatpp::mariadb::types::Status::addTransition("PENDING", "CANCELLED");
      oatpp::mariadb::types::Status::addTransition("ACTIVE", "SUSPENDED");
      oatpp::mariadb::types::Status::addTransition("ACTIVE", "COMPLETED");
      oatpp::mariadb::types::Status::addTransition("SUSPENDED", "ACTIVE");
      oatpp::mariadb::types::Status::addTransition("SUSPENDED", "CANCELLED");
      OATPP_LOGD(TAG, "Initialized valid transitions");
    }

    // Create table and verify
    {
      auto res = client.dropTable();
      OATPP_ASSERT(res->isSuccess());
      OATPP_LOGD(TAG, "Dropped existing table if any");

      res = client.createTable();
      if (!res->isSuccess()) {
        OATPP_LOGE(TAG, "Failed to create table: %s", res->getErrorMessage()->c_str());
        throw std::runtime_error("Failed to create table");
      }
      OATPP_LOGD(TAG, "Successfully created test table");
    }

    // Clear any existing data
    {
      auto res = client.deleteAll();
      OATPP_ASSERT(res->isSuccess());
      OATPP_LOGD(TAG, "Cleared existing data");
    }

    // Test cases
    {
      // Test case 1: Initial status
      {
        auto row = StatusRow::createShared();
        oatpp::mariadb::types::Status status("DRAFT");
        row->status = status.toDbValue();
        row->description = "Initial draft";
        auto res = client.insertRow(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted initial status");
      }

      // Test case 2: Valid transition
      {
        auto row = StatusRow::createShared();
        oatpp::mariadb::types::Status status("DRAFT");
        OATPP_ASSERT(status.updateStatus("PENDING"));
        row->status = status.toDbValue();
        row->description = "Pending review";
        auto res = client.insertRow(row);
        OATPP_ASSERT(res->isSuccess());
        OATPP_LOGD(TAG, "Inserted status after valid transition");
      }

      // Test case 3: Invalid status
      {
        auto row = StatusRow::createShared();
        oatpp::mariadb::types::Status status("INVALID");
        row->status = status.toDbValue();
        row->description = "Should not be inserted";
        auto res = client.insertRow(row);
        OATPP_ASSERT(!res->isSuccess());
        OATPP_LOGD(TAG, "Correctly failed to insert invalid status");
      }

      // Test case 4: Invalid transition
      {
        auto row = StatusRow::createShared();
        oatpp::mariadb::types::Status status("DRAFT");
        OATPP_ASSERT(!status.updateStatus("COMPLETED"));
        OATPP_LOGD(TAG, "Correctly prevented invalid transition");
      }
    }

    // Verify all test cases
    {
      auto res = client.selectAll();
      OATPP_ASSERT(res->isSuccess());
      
      auto dataset = res->fetch<oatpp::Vector<oatpp::Object<StatusRow>>>();
      OATPP_ASSERT(dataset->size() == 2);
      OATPP_LOGD(TAG, "Fetched %d rows from database", dataset->size());

      // Print results
      oatpp::parser::json::mapping::ObjectMapper om;
      om.getSerializer()->getConfig()->useBeautifier = true;
      OATPP_LOGD(TAG, "Setting up ObjectMapper with beautifier enabled");

      // Log each row before serialization
      for(size_t i = 0; i < dataset->size(); i++) {
        auto& row = dataset[i];
        OATPP_LOGD(TAG, "Row[%d] status: %s, description: %s", 
                   i, row->status->c_str(), row->description->c_str());
      }

      OATPP_LOGD(TAG, "Attempting to serialize dataset");
      auto str = om.writeToString(dataset);
      OATPP_LOGD(TAG, "Serialization successful. Result:\n%s", str->c_str());

      // Verify test case 1: Initial status
      {
        auto row = dataset[0];
        OATPP_ASSERT(row->status == "DRAFT");
        OATPP_ASSERT(row->description == "Initial draft");
      }

      // Verify test case 2: Valid transition
      {
        auto row = dataset[1];
        OATPP_ASSERT(row->status == "PENDING");
        OATPP_ASSERT(row->description == "Pending review");
      }

      // Additional type-specific verifications
      {
        // Verify ENUM type generation
        oatpp::mariadb::types::Status status("DRAFT");
        auto dbType = status.getDbType();
        OATPP_ASSERT(dbType->find("ENUM") != std::string::npos);
        OATPP_ASSERT(dbType->find("'DRAFT'") != std::string::npos);
        OATPP_ASSERT(dbType->find("'PENDING'") != std::string::npos);

        // Verify constraints
        auto constraints = status.getDbConstraints();
        OATPP_ASSERT(constraints == "NOT NULL");

        // Verify transitions
        auto transitions = oatpp::mariadb::types::Status::getAllowedTransitions("ACTIVE");
        OATPP_ASSERT(transitions.find("SUSPENDED") != transitions.end());
        OATPP_ASSERT(transitions.find("COMPLETED") != transitions.end());
        OATPP_ASSERT(transitions.find("CANCELLED") == transitions.end());
      }

      OATPP_LOGD(TAG, "All assertions passed successfully");
    }

    // Cleanup
    {
      auto res = client.dropTable();
      OATPP_ASSERT(res->isSuccess());
      OATPP_LOGD(TAG, "Cleaned up test table");
    }

  } catch (const std::exception& e) {
    OATPP_LOGE(TAG, "An error occurred: %s", e.what());
    throw;
  }
}

}}}} // namespace oatpp::test::mariadb::types 