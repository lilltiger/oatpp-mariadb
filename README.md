# oatpp-mariadb

This library provides integration with MariaDB database using [oatpp](https://github.com/oatpp/oatpp) framework.

MariaDB is a community-developed, commercially supported fork of the MySQL relational database management system. This library is compatible with both MariaDB and MySQL databases.

## Overview

The library provides:
- Connection pool management with deadlock protection
- Database connection configuration via options or environment
- SQL template parsing and validation
- Object-Relational Mapping (ORM) with comprehensive type support
- Schema migration with version tracking
- Transaction management with automatic retries
- Enhanced CRUD operations with metadata support
- Comprehensive data type support:
  - Numeric: Int32, Int64, UInt8, Float64
  - String: VarChar, Text, Binary (BLOB)
  - Temporal: Date, DateTime, Time, Year
  - Complex: JSON, Enum, Set
  - Generic: AnyType for flexible type handling
- Optimized query result handling:
  - State tracking for fetch operations
  - Result caching for improved performance
  - Automatic memory management and cleanup

## Type System

### Explicit Type Mapping
The library provides explicit SQL type mapping macros for improved type safety:

```cpp
// Using explicit type mappings
QUERY(createUser,
    "INSERT INTO users (name, balance, age, is_active) VALUES (:name, :balance, :age, :is_active);",
    PARAM_VARCHAR(name, 255),        // Explicit VARCHAR with length
    PARAM_DECIMAL(balance, 10, 2),   // Explicit DECIMAL with precision and scale
    PARAM_INT(age),                  // Maps to INT
    PARAM_BOOL(is_active))           // Maps to BOOLEAN
```

### Supported Type Mappings
- `PARAM_VARCHAR(name, length)` - VARCHAR with length specification
- `PARAM_DECIMAL(name, precision, scale)` - DECIMAL with precision and scale
- `PARAM_INT(name)` - Integer type
- `PARAM_BIGINT(name)` - 64-bit integer type
- `PARAM_BOOL(name)` - Boolean type
- `PARAM_TEXT(name)` - TEXT type
- `PARAM_DATETIME(name)` - DATETIME type
- `PARAM_DATE(name)` - DATE type
- `PARAM_TIME(name)` - TIME type
- `PARAM_BLOB(name)` - Binary data type

### Type Wrappers

The library provides type wrapper classes for enhanced data validation and normalization:

#### Email Type Wrapper
```cpp
/* Define an email field with validation */
DTO_FIELD(Email, email);  // Validates email format and normalizes to lowercase

/* Using the Email wrapper */
Email email("Test@Example.COM");
OATPP_ASSERT(email.normalize() == "test@example.com");  // Normalizes to lowercase

/* Validation with context */
ValidationContext context;
context.isStrict = true;           // Enable strict validation
context.allowNull = false;         // Disallow null values
context.normalizeValues = true;    // Auto-normalize during validation

OATPP_ASSERT(email.validate(context));  // Validates format and normalizes
```

#### Phone Number Type Wrapper
```cpp
/* Define a phone field with validation */
DTO_FIELD(PhoneNumber, phone);  // Validates international format

/* Using the PhoneNumber wrapper */
PhoneNumber phone(" +1-555-123-4567 ");
OATPP_ASSERT(phone.normalize() == "+1-555-123-4567");  // Removes whitespace

/* Database operations with type wrappers */
auto row = TypeWrapperRow::createShared();
Email email("Test@Example.com");
PhoneNumber phone("+1-555-123-4567");

row->email = email.toDbValue();     // Stores normalized value
row->phone = phone.toDbValue();     // Stores normalized value

client.insertRow(row);
```

#### Flag Type Wrapper
```cpp
/* Register flag values */
Flag::registerFlag("READ", 1);
Flag::registerFlag("WRITE", 2);
Flag::registerFlag("EXECUTE", 4);
Flag::registerFlag("ADMIN", 8);

/* Using Flag type in DTO */
DTO_FIELD(Flag, permissions);  // Stores as BIGINT UNSIGNED

/* Basic flag operations */
Flag flags;
flags.setFlag<std::string>("READ");      // Set by name
flags.setFlag<v_uint64>(2);              // Set by value
flags.hasFlag<std::string>("WRITE");     // Check by name
flags.hasFlag<v_uint64>(4);              // Check by value
flags.clearFlag<std::string>("READ");    // Clear by name
flags.toggleFlag<std::string>("ADMIN");  // Toggle by name

/* String conversion */
auto str = flags.toString();             // "WRITE|EXECUTE"
auto flags2 = Flag::fromString(str);     // Parse from string

/* Database operations */
auto row = FlagRow::createShared();
row->permissions = flags.toDbValue();    // Store in database
client.insertRow(row);
```

#### Features
- **Validation**: Built-in format validation using regular expressions
- **Normalization**: Automatic value normalization (e.g., lowercase emails)
- **Database Integration**: Seamless integration with database operations
- **Flexible Validation**: Context-based validation with configurable rules
- **Error Handling**: Descriptive validation error messages
- **Type Safety**: Compile-time type checking and validation
- **Length Constraints**: Automatic length validation for database compatibility
- **Bit Field Operations**: Efficient bit manipulation for flag values
- **Named Flags**: String-based flag operations with automatic lookup
- **Multiple Combinations**: Support for multiple flags in single value
- **String Serialization**: Convert to/from human-readable format
- **Database Integration**: Stores as BIGINT UNSIGNED for efficiency
- **Type Safety**: Compile-time type checking for flag operations
- **Validation**: Automatic validation of flag values and operations

#### Custom Type Wrappers
You can create custom type wrappers by inheriting from `MariaDBTypeWrapper`:

```cpp
class CustomType : public MariaDBTypeWrapper<CustomType, oatpp::String> {
public:
    explicit CustomType(const oatpp::String& value) 
        : MariaDBTypeWrapper<CustomType, oatpp::String>(value) {}
    
    bool validate() const override {
        // Your validation logic
    }
    
    oatpp::String normalize() const override {
        // Your normalization logic
    }
    
    oatpp::String getTypeName() const override {
        return oatpp::String("CustomType");
    }
};
```

#### Database Schema Integration
Type wrappers automatically generate appropriate database constraints:

```cpp
/* Email field with validation constraint */
email VARCHAR(255) NOT NULL CHECK (
    email REGEXP '^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$'
)

/* Phone field with format constraint */
phone VARCHAR(20) NOT NULL CHECK (
    phone REGEXP '^\+[0-9]{1,3}-[0-9]{3}-[0-9]{3}-[0-9]{4}$'
)
```

### Status Type
The library provides a Status type for handling operation results and error states:

```cpp
/* Using Status type */
Status status = operation.execute();
if (status.isError()) {
    // Handle error case
    auto errorCode = status.code;
    auto message = status.message;
}

/* Predefined status values */
Status::OK();              // Success status
Status::ERROR("message");  // Error status with message
```

### Flag Type
The `Flag` type provides a type-safe way to handle bit fields in MariaDB using the native BIT type. It supports configurable sizes from 1 to 64 bits and provides named flag operations with inheritance support.

```cpp
// Define a permission flag with 8 bits
using Permission = oatpp::mariadb::types::Flag<8>;

// Register flags and their values
Permission::registerFlag("READ", 1);
Permission::registerFlag("WRITE", 2);
Permission::registerFlag("EXECUTE", 4);
Permission::registerFlag("ALL", 7);

// Set up flag inheritance (ALL grants READ, WRITE, and EXECUTE)
Permission::registerFlagInheritance("ALL", "READ");
Permission::registerFlagInheritance("ALL", "WRITE");
Permission::registerFlagInheritance("ALL", "EXECUTE");

// Create and manipulate flags
Permission flags;
flags.setFlag("READ");                // Set single flag
flags.setFlagWithInheritance("ALL"); // Set flag and all inherited flags
flags.clearFlag("READ");             // Clear specific flag
flags.toggleFlag("EXECUTE");         // Toggle flag state

// Check flag states
bool canWrite = flags.hasFlag("WRITE");
bool canExecute = flags.hasFlag("EXECUTE");

// Convert to/from string representation
auto str = flags.toString();           // Returns "WRITE|EXECUTE"
auto parsed = Permission::fromString(str);

// Use in DTO
class UserDTO : public oatpp::DTO {
    DTO_INIT(UserDTO, DTO);
    DTO_FIELD(Permission, permissions);
};
```

Key Features:
- Type-safe bit field operations with compile-time validation
- Named flag registration and lookup system
- Hierarchical permissions through flag inheritance
- Automatic value validation based on bit size
- Proper database integration with endianness handling
- Memory-safe MYSQL_BIND operations
- Null value support
- String serialization for human-readable format
- Full integration with oatpp's DTO system

The Flag type automatically validates values based on the specified bit size and provides clear error messages when values exceed the maximum allowed value. It seamlessly integrates with MariaDB's native BIT type for efficient storage and retrieval.

## Dependencies

- [oatpp](https://github.com/oatpp/oatpp) - Version 1.3.0 or higher
- MariaDB/MySQL C Connector - Version 3.0.0 or higher
- CMake - Version 3.1 or higher

## Installation

### Using CMake

```bash
mkdir build && cd build
cmake ..
make
make install
```

## Configuration

### Connection Options

The library supports various connection options including:

```cpp
auto options = oatpp::mariadb::ConnectionOptions();
options.host = "127.0.0.1";           // Database host
options.port = 3306;                  // Port number
options.username = "user";            // Username
options.password = "password";        // Password
options.database = "test";            // Database name
```

### Environment Variables

You can also configure the connection using environment variables in a `.env` file:

```env
MARIADB_HOST=127.0.0.1
MARIADB_PORT=3306
MARIADB_USER=user
MARIADB_PASSWORD=password
MARIADB_DATABASE=test
```

## Usage Examples

### Basic Query Execution
```cpp
#include "oatpp-mariadb/orm.hpp"

/* Create connection provider */
auto connectionProvider = std::make_shared<oatpp::mariadb::ConnectionProvider>(options);

/* Create database client */
auto client = MyClient(std::make_shared<oatpp::mariadb::Executor>(connectionProvider));

/* Execute query */
auto result = client.selectAllRows();
```

### Schema Migration
```cpp
#include OATPP_CODEGEN_BEGIN(DTO)

/* Define version tracking DTO */
class SchemaVersionDTO : public oatpp::DTO {
  DTO_INIT(SchemaVersionDTO, DTO)
  DTO_FIELD(oatpp::Int64, version);           // Schema version
  DTO_FIELD(String, name);                    // Migration name
  DTO_FIELD(String, script);                  // Migration script
  DTO_FIELD(String, applied_at);              // Application timestamp
};

#include OATPP_CODEGEN_END(DTO)

/* Create migration table */
QUERY(createSchemaVersionTable,
      "CREATE TABLE IF NOT EXISTS `schema_version` ("
      "  `version` BIGINT NOT NULL,"
      "  `name` VARCHAR(255) NOT NULL,"
      "  `script` TEXT NOT NULL,"
      "  `applied_at` DATETIME DEFAULT CURRENT_TIMESTAMP,"
      "  PRIMARY KEY (`version`)"
      ")")
```

### Transaction Management
```cpp
/* Use TransactionGuard for automatic retry on deadlocks */
{
  auto guard = client.transactionGuard();
  try {
    // Your transaction operations here
    guard.commit();
  } catch (...) {
    guard.rollback();
  }
}
```

### Enhanced CRUD Operations
```cpp
/* Define a product DTO with metadata */
class ProductDTO : public oatpp::DTO {
  DTO_INIT(ProductDTO, DTO)
  DTO_FIELD(String, name);
  DTO_FIELD(Float64, price);
  DTO_FIELD(Object<JsonDTO>, metadata);    // JSON metadata support
};

/* Create with metadata */
QUERY(createProduct,
      "INSERT INTO products (name, price, metadata) "
      "VALUES (:product.name, :product.price, :product.metadata)")

/* Search with filters */
QUERY(findProducts,
      "SELECT * FROM products "
      "WHERE name LIKE :namePattern "
      "AND metadata->>'$.category' = :category")
```

## Testing

The library includes comprehensive tests for:
- Data type handling (numeric, string, temporal, complex types)
- Transaction management and deadlock protection
- Schema migration and version tracking
- Enhanced CRUD operations with metadata
- Connection pool management
- SQL template parsing
- JSON operations

To run the tests:

```bash
./build.sh          # Build the project
cd build/test
./oatpp-mariadb-tests
```

## Contributing

1. Fork the repository
2. Create a feature branch
3. Commit your changes
4. Push to the branch
5. Create a Pull Request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Changelog

See [CHANGELOG.md](CHANGELOG.md) for a list of changes and version history.

### Test Structure
All type tests follow a consistent pattern:

```cpp
class TypeTest : public oatpp::test::UnitTest {
private:
    /* Test configuration and setup */
    void onRun() override {
        /* Database connection setup */
        
        /* Test cases */
        testValueMapping();
        testSerialization();
        testDeserialization();
        testEdgeCases();
        
        /* Cleanup */
    }
};
```
