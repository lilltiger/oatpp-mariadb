set(OATPP_THIS_MODULE_SOURCES 
        oatpp-mariadb/mapping/Deserializer.cpp
        oatpp-mariadb/mapping/Deserializer.hpp
        oatpp-mariadb/mapping/ResultMapper.cpp
        oatpp-mariadb/mapping/ResultMapper.hpp
        oatpp-mariadb/mapping/Serializer.cpp
        oatpp-mariadb/mapping/Serializer.hpp
        oatpp-mariadb/mapping/JsonHelper.hpp
        oatpp-mariadb/ql_template/Parser.cpp
        oatpp-mariadb/ql_template/Parser.hpp
        oatpp-mariadb/ql_template/TemplateValueProvider.cpp
        oatpp-mariadb/ql_template/TemplateValueProvider.hpp
        oatpp-mariadb/Connection.cpp
        oatpp-mariadb/Connection.hpp
        oatpp-mariadb/ConnectionProvider.cpp
        oatpp-mariadb/ConnectionProvider.hpp
        oatpp-mariadb/ConnectionPool.hpp
        oatpp-mariadb/Executor.cpp
        oatpp-mariadb/Executor.hpp
        oatpp-mariadb/QueryResult.cpp
        oatpp-mariadb/QueryResult.hpp
        oatpp-mariadb/TransactionGuard.hpp
        oatpp-mariadb/orm.hpp
        oatpp-mariadb/types/MariaDBTypeWrapper.hpp
        oatpp-mariadb/types/MariaDBTypeWrapper.cpp
        oatpp-mariadb/types/CommonTypes.hpp
        oatpp-mariadb/types/SqlGenerator.hpp
        oatpp-mariadb/types/Status.hpp
        oatpp-mariadb/types/Status.cpp
        oatpp-mariadb/types/Flag.hpp
        oatpp-mariadb/types/Flag.cpp
)

add_library(${OATPP_THIS_MODULE_NAME} SHARED ${OATPP_THIS_MODULE_SOURCES})

set_target_properties(${OATPP_THIS_MODULE_NAME} PROPERTIES
        CXX_STANDARD 17
        CXX_EXTENSIONS OFF
        CXX_STANDARD_REQUIRED ON
)

if(OATPP_MODULES_LOCATION STREQUAL OATPP_MODULES_LOCATION_EXTERNAL)
    add_dependencies(${OATPP_THIS_MODULE_NAME} ${LIB_OATPP_EXTERNAL})
endif()

target_link_oatpp(${OATPP_THIS_MODULE_NAME})

target_include_directories(${OATPP_THIS_MODULE_NAME}
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        PUBLIC /usr/local/include/oatpp-1.3.0
        PUBLIC /usr/local/include/oatpp-1.3.0/oatpp
)

## Link dependencies
target_link_libraries(${OATPP_THIS_MODULE_NAME} 
    PRIVATE mariadb
)

#######################################################################################################
## install targets

if(OATPP_INSTALL)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/oatpp-mariadb
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
            COMPONENT headers
            FILES_MATCHING PATTERN "*.hpp"
    )
endif()
