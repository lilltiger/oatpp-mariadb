add_executable(oatpp-mariadb-tests
        oatpp-mariadb/ql_template/ParserTest.hpp
        oatpp-mariadb/ql_template/ParserTest.cpp
        oatpp-mariadb/types/NumericTest.hpp
        oatpp-mariadb/types/NumericTest.cpp
        oatpp-mariadb/types/Int64Test.hpp
        oatpp-mariadb/types/Int64Test.cpp
        oatpp-mariadb/types/Float64Test.hpp
        oatpp-mariadb/types/Float64Test.cpp
        oatpp-mariadb/types/UInt8Test.hpp
        oatpp-mariadb/types/UInt8Test.cpp
        oatpp-mariadb/types/DateTest.hpp
        oatpp-mariadb/types/DateTest.cpp
        oatpp-mariadb/types/DateTimeTest.hpp
        oatpp-mariadb/types/DateTimeTest.cpp
        oatpp-mariadb/types/VarCharTest.hpp
        oatpp-mariadb/types/VarCharTest.cpp
        oatpp-mariadb/types/BooleanTest.hpp
        oatpp-mariadb/types/BooleanTest.cpp
        oatpp-mariadb/types/ReturningTest.hpp
        oatpp-mariadb/types/ReturningTest.cpp
        oatpp-mariadb/types/TimeTest.hpp
        oatpp-mariadb/types/TimeTest.cpp
        oatpp-mariadb/types/YearTest.hpp
        oatpp-mariadb/types/YearTest.cpp
        oatpp-mariadb/types/EnumTest.hpp
        oatpp-mariadb/types/EnumTest.cpp
        oatpp-mariadb/types/SetTest.hpp
        oatpp-mariadb/types/SetTest.cpp
        oatpp-mariadb/types/JsonTest.hpp
        oatpp-mariadb/types/JsonTest.cpp
        oatpp-mariadb/transaction/TransactionTest.hpp
        oatpp-mariadb/transaction/TransactionTest.cpp
        oatpp-mariadb/crud/CrudTest.hpp
        oatpp-mariadb/crud/CrudTest.cpp
        oatpp-mariadb/crud/ProductCrudTest.hpp
        oatpp-mariadb/crud/ProductCrudTest.cpp
        oatpp-mariadb/tests.cpp
)

set_target_properties(oatpp-mariadb-tests PROPERTIES
        CXX_STANDARD 11
        CXX_EXTENSIONS OFF
        CXX_STANDARD_REQUIRED ON
)

target_include_directories(oatpp-mariadb-tests
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
        PUBLIC /usr/local/include/oatpp-1.3.0
        PUBLIC /usr/local/include/oatpp-1.3.0/oatpp
)

if(OATPP_MODULES_LOCATION STREQUAL OATPP_MODULES_LOCATION_EXTERNAL)
    add_dependencies(oatpp-mariadb-tests ${LIB_OATPP_EXTERNAL})
endif()

add_dependencies(oatpp-mariadb-tests ${OATPP_THIS_MODULE_NAME})

target_link_oatpp(oatpp-mariadb-tests)

target_link_libraries(oatpp-mariadb-tests
        PRIVATE ${OATPP_THIS_MODULE_NAME}
)

## TODO link dependencies here (if some)

add_test(oatpp-mariadb-tests oatpp-mariadb-tests)