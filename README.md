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
