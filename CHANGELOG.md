# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Added automatic table creation in NumericTest to improve test reliability
- Added enhanced JSON serialization configuration for better output formatting
- Added debug logging in EnvLoader for better troubleshooting

### Changed
- Updated EnvLoader to support multiple .env file locations
- Modified NumericTest to use root credentials by default
- Improved error handling in database connection code
- Enhanced logging output for better readability
- Moved ResultData to ResultMapper and improved query result handling
- Simplified QueryResult class by removing internal buffer management

### Fixed
- Fixed JSON output formatting in NumericTest
- Fixed table creation timing in tests
- Fixed environment variable loading from .env file
- Fixed memory management in QueryResult by using proper RAII patterns
- Improved error handling in statement execution
- Fixed savepoint handling in nested transactions to align with MySQL/MariaDB behavior
- Fixed transaction state management in nested transactions

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
