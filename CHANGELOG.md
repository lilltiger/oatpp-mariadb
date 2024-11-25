# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Added automatic table creation in NumericTest to improve test reliability
- Added enhanced JSON serialization configuration for better output formatting
- Added debug logging in EnvLoader for better troubleshooting
- Added TransactionGuard class for deadlock protection and automatic retries
- Added configurable MAX_RETRIES in environment settings
- Added support for nested transactions with savepoints
- Added improved string handling in TransactionGuard error messages
- Added comprehensive test summary generation in run.sh with detailed statistics
- Added detailed error categorization in test summary (runtime, test, and buffer errors)
- Added metadata JSON field support in ProductCrudTest
- Added search and filter operations in ProductCrudTest
- Added transaction support with proper isolation levels in ProductCrudTest
- Added ObjectMapper with JSON helper setup for improved serialization
- Added unique index on name field in product table
- Added new test cases for DateTime operations with timezone handling
- Added comprehensive Binary data type tests with BLOB handling
- Added enhanced JSON type validation in JsonTest
- Added Year type support with valid range checks
- Added Set type operations with multiple value handling
- Added enhanced String operations testing with different character sets
- Added Int32 specific test cases for 32-bit integer operations

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

## [1.1.0] - 2024-XX-XX
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

## [1.0.0] - 2024-11-22
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
