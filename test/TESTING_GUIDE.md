# oatpp-mariadb Testing Guide

This guide explains how to write tests for the oatpp-mariadb library, including best practices and common patterns.

## Table of Contents
1. [Setting Up Database Connection](#setting-up-database-connection)
2. [Defining DTOs](#defining-dtos)
3. [Creating Database Clients](#creating-database-clients)
4. [Working with Types](#working-with-types)
5. [Transaction Handling](#transaction-handling)
6. [Error Handling](#error-handling)
7. [Type Mappings and Usage](#type-mappings-and-usage)

## Setting Up Database Connection

### 1. Basic Connection Setup
```cpp
auto env = utils::EnvLoader();
  
auto options = oatpp::mariadb::ConnectionOptions();
options.host = env.get("MARIADB_HOST", "127.0.0.1");
options.port = env.getInt("MARIADB_PORT", 3306);
options.username = env.get("MARIADB_USER", "root");
options.password = env.get("MARIADB_PASSWORD", "root");
options.database = env.get("MARIADB_DATABASE", "test");

auto connectionProvider = std::make_shared<oatpp::mariadb::ConnectionProvider>(options);
auto executor = std::make_shared<oatpp::mariadb::Executor>(connectionProvider);
```

### 2. Environment Variables
Create a `.env` file in the project root with:
```
MARIADB_HOST=127.0.0.1
MARIADB_PORT=3306
MARIADB_USER=root
MARIADB_PASSWORD=root
MARIADB_DATABASE=test
```

## Defining DTOs

### 1. Basic DTO Structure
```cpp
#include OATPP_CODEGEN_BEGIN(DTO)

class MyDTO : public oatpp::DTO {
  DTO_INIT(MyDTO, DTO);
  
  // Define fields
  DTO_FIELD(Int32, id);
  DTO_FIELD(String, name, "name");  // "name" is the database column name
  DTO_FIELD(Boolean, active);
};

#include OATPP_CODEGEN_END(DTO)
```

### 2. Common Field Types
- `Int32` - INTEGER
- `Int64` - BIGINT
- `Float32` - FLOAT
- `Float64` - DOUBLE
- `Boolean` - BOOLEAN/TINYINT
- `String` - VARCHAR/TEXT
- `Vector<T>` - For arrays/lists
- `Object<T>` - For nested objects

## Creating Database Clients

### 1. Basic Client Structure
```cpp
#include OATPP_CODEGEN_BEGIN(DbClient)

class MyClient : public oatpp::orm::DbClient {
public:
  MyClient(const std::shared_ptr<oatpp::orm::Executor>& executor)
    : oatpp::orm::DbClient(executor)
  {
    setEnabledInterpretations({"DEFAULT", "POSTGRES"});
  }

  // Define queries
  QUERY(createTable,
        "CREATE TABLE IF NOT EXISTS `my_table` ("
        "`id` INTEGER PRIMARY KEY AUTO_INCREMENT,"
        "`name` VARCHAR(255)"
        ");")

  QUERY(insertRow,
        "INSERT INTO my_table (name) VALUES (:name);",
        PARAM(oatpp::String, name))

  QUERY(selectAll,
        "SELECT * FROM my_table;")
};

#include OATPP_CODEGEN_END(DbClient)
```

### 2. Query Parameters
- Use `:paramName` for named parameters
- Use `PARAM(type, name)` to define parameters
- For DTOs, use `PARAM(oatpp::Object<MyDTO>, dto)`

## Working with Types

### 1. Numeric Types
```cpp
// Int64 example
DTO_FIELD(Int64, signed_value);    // BIGINT
DTO_FIELD(UInt64, unsigned_value); // BIGINT UNSIGNED

// Numeric/Decimal
DTO_FIELD(Float64, price);         // DECIMAL(10,2)
```

### 2. String Types
```cpp
DTO_FIELD(String, char_value);     // CHAR(n)
DTO_FIELD(String, varchar_value);  // VARCHAR(n)
DTO_FIELD(String, text_value);     // TEXT
```

### 3. Date/Time Types
```cpp
DTO_FIELD(String, date_value);     // DATE
DTO_FIELD(String, time_value);     // TIME
DTO_FIELD(String, datetime_value); // DATETIME
```

## Transaction Handling

### 1. Using TransactionGuard
```cpp
{
  oatpp::mariadb::TransactionGuard transaction(connection);
  
  // Perform operations
  client.insertRow("value1");
  client.insertRow("value2");
  
  // Transaction automatically commits if no exceptions
  // Automatically rolls back if an exception occurs
}
```

### 2. Manual Transaction Control
```cpp
auto connection = executor->getConnection();
executor->begin(connection);

try {
    // Perform operations
    client.insertRow("value1");
    client.insertRow("value2");
    
    executor->commit(connection);
} catch (...) {
    executor->rollback(connection);
    throw;
}
```

### 3. Setting Isolation Level
```cpp
QUERY(setIsolationLevel,
      "SET SESSION TRANSACTION ISOLATION LEVEL READ COMMITTED;")
```

## Error Handling

### 1. Query Result Checking
```cpp
auto result = client.insertRow("test");
if (!result->isSuccess()) {
    OATPP_LOGE(TAG, "Query failed: %s", result->getErrorMessage()->c_str());
    throw std::runtime_error(result->getErrorMessage()->c_str());
}
```

### 2. Connection Error Handling
```cpp
try {
    auto connection = connectionProvider->get();
    if (!connection) {
        OATPP_LOGE(TAG, "Failed to establish database connection");
        throw std::runtime_error("Database connection failed");
    }
} catch (const std::exception& e) {
    OATPP_LOGE(TAG, "Database error: %s", e.what());
    throw;
}
```

### 3. Using OATPP_ASSERT in Tests
```cpp
auto result = client.selectAll();
OATPP_ASSERT(result->isSuccess());
auto rows = result->fetch<oatpp::Vector<oatpp::Object<MyDTO>>>();
OATPP_ASSERT(rows->size() > 0);
```

## Type Mappings and Usage

This section provides detailed mappings between C++ types and MariaDB types, based on the test implementations.

### Integer Types
```cpp
// Int64Test
DTO_FIELD(Int64, signed_value);     // BIGINT
DTO_FIELD(UInt64, unsigned_value);  // BIGINT UNSIGNED

// Int32Test
DTO_FIELD(Int32, signed_value);     // INT
DTO_FIELD(UInt32, unsigned_value);  // INT UNSIGNED

// UInt8Test
DTO_FIELD(Int8, signed_value);      // TINYINT
DTO_FIELD(UInt8, unsigned_value);   // TINYINT UNSIGNED
```

### Floating Point Types
```cpp
// Float64Test
DTO_FIELD(Float64, double_value);   // DOUBLE

// NumericTest
DTO_FIELD(Float64, numeric_value);  // DECIMAL(10,2)
```

### String and Text Types
```cpp
// VarCharTest
DTO_FIELD(String, varchar_value);   // VARCHAR(255)
DTO_FIELD(String, text_value);      // TEXT

// StringTest
DTO_FIELD(String, char_value);      // CHAR(64)
```

### Date and Time Types
```cpp
// DateTimeTest
DTO_FIELD(String, datetime_value);  // DATETIME

// DateTest
DTO_FIELD(String, date_value);      // DATE

// TimeTest
DTO_FIELD(String, time_value);      // TIME

// YearTest
DTO_FIELD(String, year_value);      // YEAR
```

### Binary Types
```cpp
// BinaryTest
DTO_FIELD(String, binary_value);    // BINARY(16)

// BlobTest
DTO_FIELD(String, blob_value);      // BLOB
```

### Special Types
```cpp
// JsonTest
DTO_FIELD(String, json_value);      // JSON

// EnumTest
DTO_FIELD(String, enum_value);      // ENUM('value1', 'value2', ...)

// SetTest
DTO_FIELD(String, set_value);       // SET('value1', 'value2', ...)
```

### Type Usage Examples

#### 1. Integer Example (Int64)
```cpp
#include OATPP_CODEGEN_BEGIN(DTO)

class Int64Row : public oatpp::DTO {
  DTO_INIT(Int64Row, DTO);
  DTO_FIELD(Int64, signed_value);
  DTO_FIELD(UInt64, unsigned_value);
};

#include OATPP_CODEGEN_END(DTO)

// In your database schema:
"CREATE TABLE IF NOT EXISTS `test_int64` ("
"`signed_value` BIGINT,"
"`unsigned_value` BIGINT UNSIGNED"
") ENGINE=InnoDB;"
```

#### 2. DateTime Example
```cpp
#include OATPP_CODEGEN_BEGIN(DTO)

class DateTimeRow : public oatpp::DTO {
  DTO_INIT(DateTimeRow, DTO);
  DTO_FIELD(String, datetime_value);
};

#include OATPP_CODEGEN_END(DTO)

// In your database schema:
"CREATE TABLE IF NOT EXISTS `test_datetime` ("
"`datetime_value` DATETIME"
") ENGINE=InnoDB;"

// Example usage in queries:
QUERY(insertDateTime,
      "INSERT INTO test_datetime (datetime_value) VALUES (:row.datetime_value);",
      PARAM(oatpp::Object<DateTimeRow>, row))
```

#### 3. Binary Data Example
```cpp
#include OATPP_CODEGEN_BEGIN(DTO)

class BinaryRow : public oatpp::DTO {
  DTO_INIT(BinaryRow, DTO);
  DTO_FIELD(String, binary_value);  // For BINARY(16)
};

#include OATPP_CODEGEN_END(DTO)

// In your database schema:
"CREATE TABLE IF NOT EXISTS `test_binary` ("
"`binary_value` BINARY(16)"
") ENGINE=InnoDB;"
```

### Type Conversion Notes

1. **Date/Time Types**: All date/time types are mapped to `String` in C++. The format should match MariaDB's expected format:
   - DATE: "YYYY-MM-DD"
   - TIME: "HH:MM:SS"
   - DATETIME: "YYYY-MM-DD HH:MM:SS"
   - YEAR: "YYYY"

2. **Binary Data**: Binary data is stored as a hex-encoded string in C++. Use `oatpp::utils::conversion::hex::encode/decode` for conversion.

3. **JSON Data**: JSON data is stored as a string in C++. Use `oatpp::parser::json::mapping::ObjectMapper` for conversion.

4. **Numeric Types**: For precise decimal calculations, use `DECIMAL` type in MariaDB and handle as `Float64` in C++.
