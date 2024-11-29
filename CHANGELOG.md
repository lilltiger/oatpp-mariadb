# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.3.2] - 2024-11-26

### Added
#### Data Type Tests
- Added Int32 specific test cases for 32-bit integer operations
- Added Int64Test for 64-bit integer operations and edge cases
- Added UInt8Test for unsigned 8-bit integer handling
- Added Float64Test for double precision floating point operations
- Added NumericTest with automatic table creation to improve test reliability
- Added BooleanTest for boolean data type operations and conversions
- Added VarCharTest for variable-length character string operations
- Added Binary data type tests with BLOB handling
- Added AnyType test cases for generic type handling
- Added String operations testing with different character sets

#### Date and Time Tests
- Added DateTest for date format handling and conversions
- Added DateTimeTest for timestamp and timezone operations
- Added TimeTest for time data type operations and precision
- Added Year type support with valid range checks

#### Complex Type Tests
- Added JsonTest with enhanced JSON type validation
- Added EnumTest for custom enum type validations
- Added Set type operations with multiple value handling

#### CRUD and Transaction Tests
- Added base CrudTest implementation for standard CRUD operations
- Added ProductCrudTest with advanced CRUD operations and metadata handling
- Added EnhancedCrudTest with improved CRUD patterns and validations
- Added dedicated TransactionTest for transaction lifecycle and isolation
- Added ReturningTest for testing RETURNING clause functionality
- Added TransactionGuard class for deadlock protection and automatic retries
- Added support for nested transactions with savepoints

#### Schema and Parser Tests
- Added SchemaVersionTest for database schema version management
- Added SQL template ParserTest for query template parsing and validation
- Added schema migration implementation with version tracking and validation

#### Infrastructure Improvements
- Added configurable MAX_RETRIES in environment settings
- Added ObjectMapper with JSON helper setup for improved serialization
- Added debug logging in EnvLoader for better troubleshooting
- Added comprehensive test summary generation in run.sh with detailed statistics
- Added detailed error categorization in test summary (runtime, test, and buffer errors)
- Added schema migration support with script validation and size limits
- Added hasBeenFetched tracking to QueryResult for improved fetch state management
- Implemented result caching in QueryResult for optimized data access
- Added automatic cleanup of query results to prevent memory leaks

#### Database Features
- Added unique index on name field in product table
- Added metadata JSON field support in ProductCrudTest
- Added search and filter operations in ProductCrudTest
- Added transaction support with proper isolation levels in ProductCrudTest

### Changed
- Updated EnvLoader to support multiple .env file locations
- Modified NumericTest to use root credentials by default
- Improved error handling in database connection code
- Enhanced logging output for better readability
- Moved ResultData to ResultMapper and improved query result handling
- Simplified QueryResult class by removing internal buffer management
- Updated transaction tests to use standard ORM interface for result fetching
- Enhanced TransactionGuard with more robust error handling and retry logic
- Improved transaction isolation level handling
- Improved run.sh script with better output handling and error reporting
- Enhanced test summary with categorized error reporting and improved detection
- Reverted explicit cleanup code in Int64Test and VarCharTest to resolve build errors
- Updated VarChar test cases with proper string length validation (VARCHAR(10), VARCHAR(255), TEXT(3000))
- Fixed dataset access pattern in VarChar tests
- Refactored ProductCrudTest to follow EnhancedCrudTest pattern
- Updated product table schema with updated_at timestamp
- Improved type casting in SQL queries for better type safety
- Enhanced test structure with combined transactional tests
- Modified UPDATE and DELETE queries to remove unsupported RETURNING clauses
- Enhanced Float64Test with precise decimal comparisons
- Updated DateTest with proper timezone handling
- Improved UInt8Test with boundary value testing
- Enhanced EnumTest with custom enum type validations
- Updated TimeTest with microsecond precision handling
- Improved ParserTest with complex query parsing scenarios

### Fixed
- Fixed JSON output formatting in NumericTest
- Fixed table creation timing in tests
- Fixed environment variable loading from .env file
- Fixed memory management in QueryResult by using proper RAII patterns
- Improved error handling in statement execution
- Fixed string concatenation in TransactionGuard error messages
- Fixed transaction state tracking in nested transactions
- Fixed Int64 and UInt64 serialization to properly handle buffer types and null values
- Improved memory management in Int64/UInt64 serialization methods
- Added proper handling of maximum and minimum values for 64-bit integers
- Fixed RETURNING clause usage to only use with INSERT statements in MariaDB
- Fixed DateTime precision issues in timestamp comparisons
- Fixed Binary data handling for large BLOB types
- Fixed character set handling in String operations
- Fixed Set type validation for multiple values
- Fixed Year type range validation
- Fixed JSON parsing for nested objects and arrays

## [1.3.1] - 2024-11-23
### Fixed
- Fixed transaction state tracking in QueryResult
- Improved statement cleanup with proper error handling
- Fixed transaction method implementations in Executor to properly inherit from base class
- Enhanced error handling for transaction operations

### Technical Details
- Modified `test/oatpp-mariadb/types/NumericTest.cpp`:
  - Added `createTable` query to create test tables automatically
  - Updated connection options to use root credentials
  - Improved JSON serialization configuration
  - Added better error handling and logging
- Updated `test/oatpp-mariadb/utils/EnvLoader.hpp`:
  - Added support for multiple .env file locations
  - Added debug logging for environment variable loading
  - Improved error handling for file operations

## [1.3.0] - 2024-11-22
### Added
- Improved error detection in test execution script
- Added specific error patterns for critical issues
- Enhanced log analysis with better filtering

### Changed
- Updated error analysis patterns to reduce false positives
- Refined test completion status checks
- Improved handling of binary log files

### Fixed
- Fixed false positive error detection in test logs
- Corrected pattern matching for test status messages
