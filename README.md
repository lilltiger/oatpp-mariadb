# oatpp-mariadb

This library provides integration with MariaDB database using [oatpp](https://github.com/oatpp/oatpp) framework.

MariaDB is a community-developed, commercially supported fork of the MySQL relational database management system. This library is compatible with both MariaDB and MySQL databases.

## Overview

The library provides:
- Connection pool management
- Database connection configuration
- SQL template parsing
- Object-Relational Mapping (ORM)
- Support for various data types including numeric types, dates, and strings

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

## Usage Example

```cpp
#include "oatpp-mariadb/orm.hpp"

/* Create connection provider */
auto connectionProvider = std::make_shared<oatpp::mariadb::ConnectionProvider>(options);

/* Create database client */
auto client = MyClient(std::make_shared<oatpp::mariadb::Executor>(connectionProvider));

/* Execute query */
auto result = client.selectAllRows();
```

## Testing

The library includes comprehensive tests for:
- Database connectivity
- SQL template parsing
- Data type handling
- Connection pool management

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
