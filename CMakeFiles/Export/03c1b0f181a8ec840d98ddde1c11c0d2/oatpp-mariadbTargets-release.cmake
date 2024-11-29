#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "oatpp::oatpp-mariadb" for configuration "Release"
set_property(TARGET oatpp::oatpp-mariadb APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(oatpp::oatpp-mariadb PROPERTIES
  IMPORTED_LINK_DEPENDENT_LIBRARIES_RELEASE "oatpp::oatpp;oatpp::oatpp-test"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/oatpp-1.3.0/liboatpp-mariadb.so"
  IMPORTED_SONAME_RELEASE "liboatpp-mariadb.so"
  )

list(APPEND _cmake_import_check_targets oatpp::oatpp-mariadb )
list(APPEND _cmake_import_check_files_for_oatpp::oatpp-mariadb "${_IMPORT_PREFIX}/lib/oatpp-1.3.0/liboatpp-mariadb.so" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
