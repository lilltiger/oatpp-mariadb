# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.31

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/tiger/CascadeProjects/oatpp-mariadb

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/tiger/CascadeProjects/oatpp-mariadb

# Include any dependencies generated for this target.
include test/CMakeFiles/oatpp-mariadb-tests.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include test/CMakeFiles/oatpp-mariadb-tests.dir/compiler_depend.make

# Include the progress variables for this target.
include test/CMakeFiles/oatpp-mariadb-tests.dir/progress.make

# Include the compile flags for this target's objects.
include test/CMakeFiles/oatpp-mariadb-tests.dir/flags.make

test/CMakeFiles/oatpp-mariadb-tests.dir/codegen:
.PHONY : test/CMakeFiles/oatpp-mariadb-tests.dir/codegen

test/CMakeFiles/oatpp-mariadb-tests.dir/oatpp-mariadb/tests.cpp.o: test/CMakeFiles/oatpp-mariadb-tests.dir/flags.make
test/CMakeFiles/oatpp-mariadb-tests.dir/oatpp-mariadb/tests.cpp.o: test/oatpp-mariadb/tests.cpp
test/CMakeFiles/oatpp-mariadb-tests.dir/oatpp-mariadb/tests.cpp.o: test/CMakeFiles/oatpp-mariadb-tests.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/tiger/CascadeProjects/oatpp-mariadb/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object test/CMakeFiles/oatpp-mariadb-tests.dir/oatpp-mariadb/tests.cpp.o"
	cd /home/tiger/CascadeProjects/oatpp-mariadb/test && /usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT test/CMakeFiles/oatpp-mariadb-tests.dir/oatpp-mariadb/tests.cpp.o -MF CMakeFiles/oatpp-mariadb-tests.dir/oatpp-mariadb/tests.cpp.o.d -o CMakeFiles/oatpp-mariadb-tests.dir/oatpp-mariadb/tests.cpp.o -c /home/tiger/CascadeProjects/oatpp-mariadb/test/oatpp-mariadb/tests.cpp

test/CMakeFiles/oatpp-mariadb-tests.dir/oatpp-mariadb/tests.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/oatpp-mariadb-tests.dir/oatpp-mariadb/tests.cpp.i"
	cd /home/tiger/CascadeProjects/oatpp-mariadb/test && /usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/tiger/CascadeProjects/oatpp-mariadb/test/oatpp-mariadb/tests.cpp > CMakeFiles/oatpp-mariadb-tests.dir/oatpp-mariadb/tests.cpp.i

test/CMakeFiles/oatpp-mariadb-tests.dir/oatpp-mariadb/tests.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/oatpp-mariadb-tests.dir/oatpp-mariadb/tests.cpp.s"
	cd /home/tiger/CascadeProjects/oatpp-mariadb/test && /usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/tiger/CascadeProjects/oatpp-mariadb/test/oatpp-mariadb/tests.cpp -o CMakeFiles/oatpp-mariadb-tests.dir/oatpp-mariadb/tests.cpp.s

test/CMakeFiles/oatpp-mariadb-tests.dir/oatpp-mariadb/types/TypeMappingTest.cpp.o: test/CMakeFiles/oatpp-mariadb-tests.dir/flags.make
test/CMakeFiles/oatpp-mariadb-tests.dir/oatpp-mariadb/types/TypeMappingTest.cpp.o: test/oatpp-mariadb/types/TypeMappingTest.cpp
test/CMakeFiles/oatpp-mariadb-tests.dir/oatpp-mariadb/types/TypeMappingTest.cpp.o: test/CMakeFiles/oatpp-mariadb-tests.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/tiger/CascadeProjects/oatpp-mariadb/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object test/CMakeFiles/oatpp-mariadb-tests.dir/oatpp-mariadb/types/TypeMappingTest.cpp.o"
	cd /home/tiger/CascadeProjects/oatpp-mariadb/test && /usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT test/CMakeFiles/oatpp-mariadb-tests.dir/oatpp-mariadb/types/TypeMappingTest.cpp.o -MF CMakeFiles/oatpp-mariadb-tests.dir/oatpp-mariadb/types/TypeMappingTest.cpp.o.d -o CMakeFiles/oatpp-mariadb-tests.dir/oatpp-mariadb/types/TypeMappingTest.cpp.o -c /home/tiger/CascadeProjects/oatpp-mariadb/test/oatpp-mariadb/types/TypeMappingTest.cpp

test/CMakeFiles/oatpp-mariadb-tests.dir/oatpp-mariadb/types/TypeMappingTest.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/oatpp-mariadb-tests.dir/oatpp-mariadb/types/TypeMappingTest.cpp.i"
	cd /home/tiger/CascadeProjects/oatpp-mariadb/test && /usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/tiger/CascadeProjects/oatpp-mariadb/test/oatpp-mariadb/types/TypeMappingTest.cpp > CMakeFiles/oatpp-mariadb-tests.dir/oatpp-mariadb/types/TypeMappingTest.cpp.i

test/CMakeFiles/oatpp-mariadb-tests.dir/oatpp-mariadb/types/TypeMappingTest.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/oatpp-mariadb-tests.dir/oatpp-mariadb/types/TypeMappingTest.cpp.s"
	cd /home/tiger/CascadeProjects/oatpp-mariadb/test && /usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/tiger/CascadeProjects/oatpp-mariadb/test/oatpp-mariadb/types/TypeMappingTest.cpp -o CMakeFiles/oatpp-mariadb-tests.dir/oatpp-mariadb/types/TypeMappingTest.cpp.s

# Object files for target oatpp-mariadb-tests
oatpp__mariadb__tests_OBJECTS = \
"CMakeFiles/oatpp-mariadb-tests.dir/oatpp-mariadb/tests.cpp.o" \
"CMakeFiles/oatpp-mariadb-tests.dir/oatpp-mariadb/types/TypeMappingTest.cpp.o"

# External object files for target oatpp-mariadb-tests
oatpp__mariadb__tests_EXTERNAL_OBJECTS =

test/oatpp-mariadb-tests: test/CMakeFiles/oatpp-mariadb-tests.dir/oatpp-mariadb/tests.cpp.o
test/oatpp-mariadb-tests: test/CMakeFiles/oatpp-mariadb-tests.dir/oatpp-mariadb/types/TypeMappingTest.cpp.o
test/oatpp-mariadb-tests: test/CMakeFiles/oatpp-mariadb-tests.dir/build.make
test/oatpp-mariadb-tests: test/CMakeFiles/oatpp-mariadb-tests.dir/compiler_depend.ts
test/oatpp-mariadb-tests: src/liboatpp-mariadb.so
test/oatpp-mariadb-tests: /usr/local/lib/liboatpp-test.so
test/oatpp-mariadb-tests: /usr/local/lib/liboatpp.so
test/oatpp-mariadb-tests: test/CMakeFiles/oatpp-mariadb-tests.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/home/tiger/CascadeProjects/oatpp-mariadb/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX executable oatpp-mariadb-tests"
	cd /home/tiger/CascadeProjects/oatpp-mariadb/test && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/oatpp-mariadb-tests.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
test/CMakeFiles/oatpp-mariadb-tests.dir/build: test/oatpp-mariadb-tests
.PHONY : test/CMakeFiles/oatpp-mariadb-tests.dir/build

test/CMakeFiles/oatpp-mariadb-tests.dir/clean:
	cd /home/tiger/CascadeProjects/oatpp-mariadb/test && $(CMAKE_COMMAND) -P CMakeFiles/oatpp-mariadb-tests.dir/cmake_clean.cmake
.PHONY : test/CMakeFiles/oatpp-mariadb-tests.dir/clean

test/CMakeFiles/oatpp-mariadb-tests.dir/depend:
	cd /home/tiger/CascadeProjects/oatpp-mariadb && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/tiger/CascadeProjects/oatpp-mariadb /home/tiger/CascadeProjects/oatpp-mariadb/test /home/tiger/CascadeProjects/oatpp-mariadb /home/tiger/CascadeProjects/oatpp-mariadb/test /home/tiger/CascadeProjects/oatpp-mariadb/test/CMakeFiles/oatpp-mariadb-tests.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : test/CMakeFiles/oatpp-mariadb-tests.dir/depend

