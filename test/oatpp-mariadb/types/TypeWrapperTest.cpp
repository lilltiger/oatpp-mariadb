#include "TypeWrapperTest.hpp"
#include "oatpp-mariadb/Connection.hpp"
#include "oatpp-mariadb/ConnectionProvider.hpp"
#include "oatpp/core/base/Environment.hpp"
#include "../utils/EnvLoader.hpp"

namespace oatpp { namespace test { namespace mariadb { namespace types {

namespace {
    const char* getEnvOrDefault(const char* name, const char* defaultValue) {
        const char* value = std::getenv(name);
        return value ? value : defaultValue;
    }

    int getEnvAsIntOrDefault(const char* name, int defaultValue) {
        const char* value = std::getenv(name);
        return value ? std::atoi(value) : defaultValue;
    }

    void runDatabaseTests(const std::shared_ptr<oatpp::orm::Executor>& executor) {
        auto client = TestClient(executor);
        
        // Clean start
        client.dropTable();
        client.createTable();
        
        // Test valid data insertion
        {
            auto row = TypeWrapperRow::createShared();
            Email email("Test@Example.com");
            PhoneNumber phone("+1-555-123-4567");
            row->email = email.toDbValue();  // Use normalized value
            row->phone = phone.toDbValue();  // Use normalized value
            row->name = "Test User";
            
            client.insertRow(row);
            
            auto result = client.selectAll()->fetch<oatpp::Vector<oatpp::Object<TypeWrapperRow>>>();
            OATPP_ASSERT(result->size() == 1);
            OATPP_ASSERT(result[0]->email == "test@example.com");  // Verify normalization
            OATPP_ASSERT(result[0]->phone == "+1-555-123-4567");  // Verify normalization
        }
        
        client.deleteAll();
        client.dropTable();
    }
}

void TypeWrapperTest::onRun() {
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

        // Run database tests
        runDatabaseTests(executor);
    } catch (const std::exception& e) {
        OATPP_LOGE(TAG, "Database error: %s", e.what());
        throw;
    }
    
    // 1. Test Null Handling
    {
        Email nullEmail(nullptr);
        OATPP_ASSERT(nullEmail.isNull());
        
        ValidationContext strictContext;
        strictContext.isStrict = true;
        strictContext.allowNull = false;
        OATPP_ASSERT(!nullEmail.validate(strictContext));
        
        ValidationContext lenientContext;
        lenientContext.isStrict = false;
        lenientContext.allowNull = true;
        OATPP_ASSERT(nullEmail.validate(lenientContext));
        
        auto nullEmailError = nullEmail.getValidationError();
        OATPP_ASSERT(!nullEmailError || nullEmailError->empty());
    }
    
    // 2. Test Value Normalization
    {
        Email email("Test@Example.COM");
        OATPP_ASSERT(email.normalize() == "test@example.com");
        
        PhoneNumber phone(" +1-555-123-4567 ");
        OATPP_ASSERT(phone.normalize() == "+1-555-123-4567");
        
        ValidationContext normalizeContext;
        normalizeContext.normalizeValues = true;
        OATPP_ASSERT(email.validate(normalizeContext));
        OATPP_ASSERT(phone.validate(normalizeContext));
    }
    
    // 3. Test Validation Contexts
    {
        Email email("invalid-email");
        
        ValidationContext strictContext;
        strictContext.isStrict = true;
        OATPP_ASSERT(!email.validate(strictContext));
        
        ValidationContext lenientContext;
        lenientContext.isStrict = false;
        OATPP_ASSERT(!email.validate(lenientContext));  // Still invalid even in lenient mode
        
        Email validEmail("valid@example.com");
        OATPP_ASSERT(validEmail.validate(strictContext));
    }
    
    // 4. Test Error Messages
    {
        Email invalidEmail("bad-email");
        auto emailError = invalidEmail.getValidationError();
        OATPP_ASSERT(emailError->find("Email") != std::string::npos);
        
        PhoneNumber invalidPhone("123-456-7890");
        auto phoneError = invalidPhone.getValidationError();
        OATPP_ASSERT(phoneError->find("Phone Number") != std::string::npos);
    }
    
    // 5. Test Size Constraints
    {
        std::string longEmail;
        for(int i = 0; i < 250; i++) longEmail += "a";
        longEmail += "@test.com";
        
        Email email(longEmail.c_str());
        OATPP_ASSERT(!email.validateLength());
        
        auto error = email.getValidationError();
        OATPP_ASSERT(error->find("maximum length") != std::string::npos);
    }
    
    // 6. Test Operators
    {
        Email email1("test@example.com");
        Email email2("test@example.com");
        Email email3("other@example.com");
        
        OATPP_ASSERT(email1 == email2);
        OATPP_ASSERT(email1 != email3);
        OATPP_ASSERT(email3 < email1);  // Lexicographical comparison - "other" comes before "test"
    }
    
    // 7. Test Database Type Mapping
    {
        Email email("");
        PhoneNumber phone("");
        
        OATPP_ASSERT(email.getDbType() == "VARCHAR(255)");
        OATPP_ASSERT(phone.getDbType() == "VARCHAR(20)");
        
        OATPP_ASSERT(email.getDbConstraints()->find("REGEXP") != std::string::npos);
        OATPP_ASSERT(phone.getDbConstraints()->find("REGEXP") != std::string::npos);
    }
    
    // 8. Test Serialization
    {
        Email originalEmail("Test@Example.com");
        auto dbValue = originalEmail.toDbValue();
        auto reconstructedEmail = Email::fromDbValue(dbValue);
        OATPP_ASSERT(reconstructedEmail.getValue() == "test@example.com");
    }
}

}}}} 