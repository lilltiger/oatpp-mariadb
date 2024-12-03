# Custom Type Implementation Guide for oatpp-mariadb

This guide explains how to implement custom types in oatpp-mariadb, using the Flag type as a reference implementation.

## Table of Contents
1. [Overview](#overview)
2. [Implementation Steps](#implementation-steps)
3. [Example Implementation](#example-implementation)
4. [Testing Your Type](#testing-your-type)
5. [Best Practices](#best-practices)

## Overview

Custom types in oatpp-mariadb require several components:
1. Type Definition
2. Serializer Implementation
3. Result Mapper Implementation
4. Database Type Mapping

## Implementation Steps

### 1. Define Your Type Class

Create a header file for your type (e.g., `include/oatpp-mariadb/types/YourType.hpp`):

```cpp
template<typename T>
class YourType : public oatpp::data::mapping::type::Primitive<T> {
private:
    T m_value;

public:
    // Constructor
    YourType(T value = T()) : m_value(value) {}

    // Value access methods
    T getValue(const T& defaultValue) const {
        return m_value;
    }

    void setValue(T value) {
        m_value = value;
    }

    // Type registration
    static void setupSerializer(oatpp::mariadb::mapping::Serializer& serializer) {
        // Register serialization method
    }
};
```

### 2. Implement Serialization

Create a serializer implementation (e.g., `src/oatpp-mariadb/mapping/type/YourTypeMapping.hpp`):

```cpp
namespace oatpp { namespace mariadb { namespace mapping { namespace type {

template<typename T>
class YourTypeMapping {
public:
    static void install(oatpp::mariadb::mapping::ResultMapper& mapper) {
        // Define how your type is mapped from database values
    }

    static void serializeMethod(const Serializer* serializer,
                              MYSQL_STMT* stmt,
                              v_uint32 paramIndex,
                              const oatpp::Void& polymorph) {
        // Define how your type is serialized to database values
    }
};

}}}}
```

### 3. Database Type Mapping

Choose appropriate database types for storage:

```sql
-- Example table definition
CREATE TABLE your_type_table (
    id INTEGER PRIMARY KEY AUTO_INCREMENT,
    your_field YOUR_DB_TYPE -- Choose appropriate type
);
```

## Example Implementation: Flag Type

Here's how the Flag type is implemented:

### 1. Helper Components

#### Static Storage

```cpp
template<v_uint32 N>
class Flag {
private:
    // Store flag name-to-value mappings
    static std::unordered_map<std::string, v_uint64> s_flagValues;
    
    // Store flag inheritance relationships
    static std::unordered_map<std::string, std::vector<std::string>> s_flagInheritance;
};

// Define storage outside the class
template<v_uint32 N>
std::unordered_map<std::string, v_uint64> Flag<N>::s_flagValues;

template<v_uint32 N>
std::unordered_map<std::string, std::vector<std::string>> Flag<N>::s_flagInheritance;
```

#### Flag Inheritance System

```cpp
// Register parent-child relationships between flags
static void registerFlagInheritance(const std::string& parent, const std::string& child) {
    s_flagInheritance[parent].push_back(child);
}

// Set a flag and all its inherited flags
void setFlagWithInheritance(const std::string& name) {
    setFlag(name);
    auto it = s_flagInheritance.find(name);
    if (it != s_flagInheritance.end()) {
        for (const auto& child : it->second) {
            setFlag(child);
        }
    }
}
```

#### DTO Integration

```cpp
template<v_uint32 N>
class Flag : public oatpp::UInt64 {
    DTO_INIT(Flag, UInt64)  // Integrate with oatpp's DTO system
    typedef Flag<N> __Flag;
    static const oatpp::data::mapping::type::ClassId CLASS_ID;
};

template<v_uint32 N>
const oatpp::data::mapping::type::ClassId Flag<N>::CLASS_ID("Flag");
```

### 2. Flag Type Definition (`Flag.hpp`)

```cpp
template<v_uint32 BIT_COUNT>
class Flag : public oatpp::data::mapping::type::Primitive<v_uint64> {
private:
    v_uint64 m_value;
    static std::unordered_map<oatpp::String, v_uint64> s_flags;

public:
    Flag(v_uint64 value = 0) : m_value(value) {}

    v_uint64 getValue(const v_uint64& defaultValue) const {
        return m_value;
    }

    void setValue(v_uint64 value) {
        m_value = value;
    }

    static void registerFlag(const oatpp::String& name, v_uint64 value) {
        s_flags[name] = value;
    }

    void setFlag(const oatpp::String& name) {
        auto it = s_flags.find(name);
        if(it != s_flags.end()) {
            m_value |= it->second;
        }
    }

    bool hasFlag(const oatpp::String& name) const {
        auto it = s_flags.find(name);
        if(it != s_flags.end()) {
            return (m_value & it->second) == it->second;
        }
        return false;
    }
};
```

### 3. Database Interaction

#### Type-Specific Serialization

The Flag type requires special serialization handling:

```cpp
static void setupSerializer(oatpp::mariadb::mapping::Serializer& serializer) {
    serializer.setSerializerMethod(Flag<N>::CLASS_ID,
        [](const Serializer* _this, MYSQL_STMT* stmt, v_uint32 paramIndex, const oatpp::Void& polymorph) {
            auto& bind = _this->getBindParams()[paramIndex];
            std::memset(&bind, 0, sizeof(MYSQL_BIND));
            
            if(polymorph) {
                auto value = static_cast<Flag<N>*>(polymorph.get());
                if(value) {
                    // Use BIT type for flag storage
                    bind.buffer_type = MYSQL_TYPE_BIT;
                    bind.buffer = malloc(8);  // 8 bytes for BIT(64)
                    
                    // Handle value conversion
                    v_uint64 val = value->getValue((v_uint64)0);
                    unsigned char* bytes = static_cast<unsigned char*>(bind.buffer);
                    
                    // Handle endianness
                    for(size_t i = 0; i < 8; i++) {
                        bytes[i] = (val >> (i * 8)) & 0xFF;
                    }
                    
                    bind.buffer_length = 8;
                    bind.is_null_value = 0;
                    bind.length_value = 8;
                }
            }
            
            // Set up bind parameters
            bind.is_null = &bind.is_null_value;
            bind.length = &bind.length_value;
        });
}
```

Key points:
- Uses `MYSQL_TYPE_BIT` for database storage
- Allocates 8 bytes for 64-bit flags
- Handles endianness conversion
- Properly sets up MYSQL_BIND structure

#### Type-Specific Deserialization

The Flag type also needs special deserialization handling:

```cpp
static void install(ResultMapper& mapper) {
    mapper.setReadOneRowMethod(Flag<N>::CLASS_ID,
        [](ResultMapper* _this, ResultData* dbData, const Type* type) -> oatpp::Void {
            if (!dbData->hasMore) {
                return oatpp::Void(nullptr);
            }

            auto& bind = dbData->bindResults[0];
            if (!*bind.is_null && bind.buffer && bind.buffer_length > 0) {
                // Convert from database format
                const unsigned char* bytes = static_cast<const unsigned char*>(bind.buffer);
                v_uint64 val = 0;
                
                // Handle endianness
                for(size_t i = 0; i < std::min(bind.buffer_length, size_t(8)); i++) {
                    val |= static_cast<v_uint64>(bytes[i]) << (i * 8);
                }
                
                return oatpp::Void(std::make_shared<Flag<N>>(val));
            }
            
            return oatpp::Void(nullptr);
        });
}
```

Key points:
- Handles null values
- Validates buffer contents
- Converts from database byte order
- Creates appropriate Flag instance

#### Serialization Considerations

When implementing serialization for custom types:

1. **Type Mapping**
   - Choose appropriate MySQL type (`MYSQL_TYPE_*`)
   - Consider size limitations
   - Handle type-specific requirements

2. **Memory Management**
   - Allocate appropriate buffer size
   - Clean up allocated memory
   - Handle buffer overflow protection

3. **Data Conversion**
   - Handle endianness if needed
   - Convert between C++ and MySQL types
   - Validate data ranges

4. **Null Handling**
   - Properly set null indicators
   - Handle null input values
   - Consider nullable vs non-nullable fields

### 4. Usage Example

```cpp
// Register flags and their values
Flag<64>::registerFlag("READ", 1ULL);
Flag<64>::registerFlag("WRITE", 2ULL);
Flag<64>::registerFlag("EXECUTE", 4ULL);
Flag<64>::registerFlag("ALL", 7ULL);

// Set up inheritance
Flag<64>::registerFlagInheritance("ALL", "READ");
Flag<64>::registerFlagInheritance("ALL", "WRITE");
Flag<64>::registerFlagInheritance("ALL", "EXECUTE");

// Create and use a flag
auto flag = Flag<64>::createShared();
flag->setFlagWithInheritance("ALL");  // Sets ALL and its inherited flags
```

## Testing Your Type

1. Create a test file (e.g., `test/oatpp-mariadb/types/YourTypeTest.cpp`)
2. Define test cases:
   - Basic value storage and retrieval
   - Edge cases (null values, limits)
   - Type-specific functionality
   - Error cases

Example test structure:

```cpp
class YourTypeTest : public oatpp::test::UnitTest {
public:
    void onRun() override {
        // Setup
        auto client = createTestClient();

        // Test cases
        testBasicOperations(client);
        testEdgeCases(client);
        testErrorCases(client);
    }

private:
    void testBasicOperations(const std::shared_ptr<TestClient>& client) {
        // Test basic value storage and retrieval
    }

    void testEdgeCases(const std::shared_ptr<TestClient>& client) {
        // Test edge cases
    }

    void testErrorCases(const std::shared_ptr<TestClient>& client) {
        // Test error handling
    }
};
```

## Best Practices

1. **Type Safety**
   - Use appropriate C++ types
   - Validate input values
   - Handle null values properly

2. **Database Compatibility**
   - Choose appropriate database types
   - Consider size limitations
   - Handle database-specific quirks

3. **Performance**
   - Minimize memory allocations
   - Use efficient serialization methods
   - Consider caching if appropriate

4. **Error Handling**
   - Provide clear error messages
   - Handle edge cases gracefully
   - Document limitations

5. **Testing**
   - Test all functionality
   - Include edge cases
   - Test error conditions
   - Test with large datasets

## Common Pitfalls

1. **Memory Management**
   - Ensure proper cleanup
   - Handle ownership correctly
   - Watch for memory leaks

2. **Type Conversion**
   - Be careful with implicit conversions
   - Handle overflow/underflow
   - Consider endianness

3. **Database Interaction**
   - Handle connection failures
   - Consider transaction boundaries
   - Watch for SQL injection

## Integration Tips

1. **DTO Integration**
```cpp
class MyDTO : public oatpp::DTO {
    DTO_INIT(MyDTO, DTO)
    DTO_FIELD(YourType<T>, field_name);
};
```

2. **Query Usage**
```cpp
QUERY(insertValue,
      "INSERT INTO your_table (field) VALUES (:value);",
      PARAM(YourType<T>, value))
```

3. **Value Retrieval**
```cpp
auto result = client.selectValue();
auto value = result->fetch<YourType<T>>();
```

## Conclusion

Implementing custom types requires careful consideration of:
- Type safety
- Database compatibility
- Performance
- Error handling
- Testing

Follow this guide and the example implementation to create robust and reliable custom types for your oatpp-mariadb applications. 

## Implementation Considerations

When implementing helper components like those in the Flag type:

1. **Static Storage**
   - Consider thread safety for static storage
   - Initialize static members appropriately
   - Handle cleanup if necessary

2. **Inheritance Systems**
   - Design for flexibility and extensibility
   - Consider validation of inheritance relationships
   - Handle circular dependencies
   - Document inheritance behavior

3. **DTO Integration**
   - Properly inherit from appropriate base classes
   - Define necessary type information
   - Implement required DTO macros
   - Handle serialization/deserialization

4. **Endianness**
   - Document byte order requirements
   - Handle platform-specific differences
   - Validate data during conversion
   - Consider performance implications