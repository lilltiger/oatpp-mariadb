# Common Issues in oatpp-mariadb Tests

## 1. Void Type Serialization Issues

### Problem
Error: `[oatpp::parser::json::mapping::Serializer::serialize()]: Error. No serialize method for type 'Void'`

### Solution
- Use a single String field to store complex data as JSON instead of multiple typed fields
- Let the database handle JSON validation and storage
- Follow JsonTest's pattern of simple DTOs

Example:
cpp
// Instead of multiple fields:
class CustomTypeRow : public oatpp::DTO {
DTO_FIELD(Int64, customer_id);
DTO_FIELD(String, name);
DTO_FIELD(Float64, balance);
};
// Use a single JSON field:
class CustomTypeRow : public oatpp::DTO {
DTO_FIELD(String, data); // Store as JSON string
};



## 2. Record State Between Tests

### Problem
Tests failing because previous test data affects current test

### Solution
- Clear data before each test case
- Don't rely on previous test state
- Use deleteAll() before each test

Example:
cpp
// Before each test
auto res = client.deleteAll();
OATPP_ASSERT(res->isSuccess());


## 3. Update Operations Without Existing Records

### Problem
Update operations failing because target record doesn't exist

### Solution
- Insert record before attempting update
- Use same key (e.g., customer_id) in both operations
- Verify update with select

Example:
cpp
// First insert
auto insertRow = CustomTypeRow::createShared();
insertRow->data = "{\"customer_id\":42,\"name\":\"John\"}";
res = client.insertValue(insertRow);
// Then update
auto updateRow = CustomTypeRow::createShared();
updateRow->data = "{\"customer_id\":42,\"name\":\"Jane\"}";
res = client.updateValue(updateRow);



## 4. Context Window Management

### Problem
Large error logs and file contents filling up context window, causing loss of context

### Solution
- Clear error logs when not needed
- Keep track of important information despite context limitations
- Be clear when context is being lost
- Focus on relevant code sections

## 5. Test Coverage Duplication

### Problem
Creating new tests that duplicate existing functionality

### Solution
- Check tests.cpp for existing test files
- Read through existing type tests
- Understand what's already covered
- Focus on missing type coverage
- Follow existing test patterns

## String Conversion

### oatpp::String to C-string
When working with `oatpp::String`, use `->c_str()` directly to get the C-string representation. 
Common mistakes include:
- Using `std_str()` which doesn't exist (this is a common confusion with std::string's str() method)
- Using `std::string()` unnecessarily before `c_str()`
- Trying to use string methods that belong to std::string on oatpp::String

Example:
cpp
// CORRECT:
oatpp::String str = "example";
const char* cstr = str->c_str();

// WRONG:
const char* wrong1 = str->std_str();  // Error: no member named 'std_str'
const char* wrong2 = str->std::string().c_str();  // Unnecessary conversion

// If you need std::string operations, convert explicitly:
std::string stdStr(str->c_str());  // Now you can use std::string methods

### Common Conversion Patterns
1. oatpp::String to const char*: `str->c_str()`
2. oatpp::String to std::string: `std::string(str->c_str())`
3. std::string to oatpp::String: `oatpp::String(stdStr.c_str())`

## Test Issues

### Test Database Connection
- Ensure MariaDB is running and accessible
- Check credentials in `.env` file
- Verify database exists and has proper permissions

### Test Table Creation
- Check for proper SQL syntax in CREATE TABLE statements
- Ensure proper character set and collation
- Verify table name conflicts

### Test Data Validation
- Ensure proper regex patterns for validation
- Check for proper escaping in SQL statements
- Verify constraint definitions

## Build Issues

### Missing Dependencies
- Check all required dependencies are installed
- Verify correct versions of dependencies
- Check pkg-config path is set correctly

### Compilation Errors
- Check include paths
- Verify macro definitions
- Check for proper header guards

## Runtime Issues

### Connection Pool
- Check max connections setting
- Verify connection timeout settings
- Monitor for connection leaks

### Memory Management
- Watch for proper shared_ptr usage
- Check for memory leaks
- Monitor object lifecycle

## Best Practices

1. Follow existing test patterns (JsonTest, SetTest, etc.)
2. Keep DTOs simple
3. Clear data between tests
4. Use appropriate data types
5. Verify operations with select queries
6. Add detailed logging for debugging
7. Handle NULL values explicitly
8. Test special characters and edge cases
9. Always check query results
10. Provide meaningful error messages
11. Implement proper cleanup in error cases
12. Use RAII patterns
13. Properly close connections
14. Clean up temporary resources
15. Write comprehensive unit tests
16. Include edge cases
17. Test with various data types
