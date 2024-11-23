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
- Fixed test summary generation in run.sh to handle broken pipes and newlines correctly

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
