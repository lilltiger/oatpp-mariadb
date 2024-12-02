add_executable(oatpp-mariadb-tests
        oatpp-mariadb/tests.cpp
        oatpp-mariadb/ql_template/ParserTest.cpp
        oatpp-mariadb/types/NumericTest.cpp
        oatpp-mariadb/types/Int64Test.cpp
        oatpp-mariadb/types/Float64Test.cpp
        oatpp-mariadb/types/UInt8Test.cpp
        oatpp-mariadb/types/DateTest.cpp
        oatpp-mariadb/types/DateTimeTest.cpp
        oatpp-mariadb/types/VarCharTest.cpp
        oatpp-mariadb/types/BooleanTest.cpp
        oatpp-mariadb/types/ReturningTest.cpp
        oatpp-mariadb/types/TimeTest.cpp
        oatpp-mariadb/types/YearTest.cpp
        oatpp-mariadb/types/EnumTest.cpp
        oatpp-mariadb/types/SetTest.cpp
        oatpp-mariadb/types/JsonTest.cpp
        oatpp-mariadb/types/BinaryTest.cpp
        oatpp-mariadb/types/Int32Test.cpp
        oatpp-mariadb/types/StringTest.cpp
        oatpp-mariadb/migration/SchemaVersionTest.cpp
        oatpp-mariadb/QueryResultTest.cpp
        oatpp-mariadb/transaction/TransactionTest.cpp
        oatpp-mariadb/crud/CrudTest.cpp
        oatpp-mariadb/crud/ProductCrudTest.cpp
        oatpp-mariadb/crud/EnhancedCrudTest.cpp
        oatpp-mariadb/types/TypeMappingTest.cpp
        oatpp-mariadb/types/CustomTypeTest.cpp
        oatpp-mariadb/types/MariaDBTypeWrapperTest.cpp
        oatpp-mariadb/types/StatusTest.cpp
        oatpp-mariadb/types/FlagTest.cpp
)

set_target_properties(oatpp-mariadb-tests PROPERTIES
        CXX_STANDARD 17
        CXX_EXTENSIONS OFF
        CXX_STANDARD_REQUIRED ON
)

target_include_directories(oatpp-mariadb-tests
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
        PUBLIC /usr/local/include/oatpp-1.3.0
)

if(OATPP_MODULES_LOCATION STREQUAL OATPP_MODULES_LOCATION_EXTERNAL)
    add_dependencies(oatpp-mariadb-tests ${LIB_OATPP_EXTERNAL})
endif()

add_dependencies(oatpp-mariadb-tests ${OATPP_THIS_MODULE_NAME})

target_link_oatpp(oatpp-mariadb-tests)

target_link_libraries(oatpp-mariadb-tests
        PRIVATE ${OATPP_THIS_MODULE_NAME}
)

add_test(NAME oatpp-mariadb-tests COMMAND oatpp-mariadb-tests)