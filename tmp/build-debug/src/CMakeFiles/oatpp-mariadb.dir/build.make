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
CMAKE_BINARY_DIR = /home/tiger/CascadeProjects/oatpp-mariadb/tmp/build-debug

# Include any dependencies generated for this target.
include src/CMakeFiles/oatpp-mariadb.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include src/CMakeFiles/oatpp-mariadb.dir/compiler_depend.make

# Include the progress variables for this target.
include src/CMakeFiles/oatpp-mariadb.dir/progress.make

# Include the compile flags for this target's objects.
include src/CMakeFiles/oatpp-mariadb.dir/flags.make

src/CMakeFiles/oatpp-mariadb.dir/codegen:
.PHONY : src/CMakeFiles/oatpp-mariadb.dir/codegen

src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/mapping/Deserializer.cpp.o: src/CMakeFiles/oatpp-mariadb.dir/flags.make
src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/mapping/Deserializer.cpp.o: /home/tiger/CascadeProjects/oatpp-mariadb/src/oatpp-mariadb/mapping/Deserializer.cpp
src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/mapping/Deserializer.cpp.o: src/CMakeFiles/oatpp-mariadb.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/tiger/CascadeProjects/oatpp-mariadb/tmp/build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/mapping/Deserializer.cpp.o"
	cd /home/tiger/CascadeProjects/oatpp-mariadb/tmp/build-debug/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/mapping/Deserializer.cpp.o -MF CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/mapping/Deserializer.cpp.o.d -o CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/mapping/Deserializer.cpp.o -c /home/tiger/CascadeProjects/oatpp-mariadb/src/oatpp-mariadb/mapping/Deserializer.cpp

src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/mapping/Deserializer.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/mapping/Deserializer.cpp.i"
	cd /home/tiger/CascadeProjects/oatpp-mariadb/tmp/build-debug/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/tiger/CascadeProjects/oatpp-mariadb/src/oatpp-mariadb/mapping/Deserializer.cpp > CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/mapping/Deserializer.cpp.i

src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/mapping/Deserializer.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/mapping/Deserializer.cpp.s"
	cd /home/tiger/CascadeProjects/oatpp-mariadb/tmp/build-debug/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/tiger/CascadeProjects/oatpp-mariadb/src/oatpp-mariadb/mapping/Deserializer.cpp -o CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/mapping/Deserializer.cpp.s

src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/mapping/ResultMapper.cpp.o: src/CMakeFiles/oatpp-mariadb.dir/flags.make
src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/mapping/ResultMapper.cpp.o: /home/tiger/CascadeProjects/oatpp-mariadb/src/oatpp-mariadb/mapping/ResultMapper.cpp
src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/mapping/ResultMapper.cpp.o: src/CMakeFiles/oatpp-mariadb.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/tiger/CascadeProjects/oatpp-mariadb/tmp/build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/mapping/ResultMapper.cpp.o"
	cd /home/tiger/CascadeProjects/oatpp-mariadb/tmp/build-debug/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/mapping/ResultMapper.cpp.o -MF CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/mapping/ResultMapper.cpp.o.d -o CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/mapping/ResultMapper.cpp.o -c /home/tiger/CascadeProjects/oatpp-mariadb/src/oatpp-mariadb/mapping/ResultMapper.cpp

src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/mapping/ResultMapper.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/mapping/ResultMapper.cpp.i"
	cd /home/tiger/CascadeProjects/oatpp-mariadb/tmp/build-debug/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/tiger/CascadeProjects/oatpp-mariadb/src/oatpp-mariadb/mapping/ResultMapper.cpp > CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/mapping/ResultMapper.cpp.i

src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/mapping/ResultMapper.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/mapping/ResultMapper.cpp.s"
	cd /home/tiger/CascadeProjects/oatpp-mariadb/tmp/build-debug/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/tiger/CascadeProjects/oatpp-mariadb/src/oatpp-mariadb/mapping/ResultMapper.cpp -o CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/mapping/ResultMapper.cpp.s

src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/mapping/Serializer.cpp.o: src/CMakeFiles/oatpp-mariadb.dir/flags.make
src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/mapping/Serializer.cpp.o: /home/tiger/CascadeProjects/oatpp-mariadb/src/oatpp-mariadb/mapping/Serializer.cpp
src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/mapping/Serializer.cpp.o: src/CMakeFiles/oatpp-mariadb.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/tiger/CascadeProjects/oatpp-mariadb/tmp/build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/mapping/Serializer.cpp.o"
	cd /home/tiger/CascadeProjects/oatpp-mariadb/tmp/build-debug/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/mapping/Serializer.cpp.o -MF CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/mapping/Serializer.cpp.o.d -o CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/mapping/Serializer.cpp.o -c /home/tiger/CascadeProjects/oatpp-mariadb/src/oatpp-mariadb/mapping/Serializer.cpp

src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/mapping/Serializer.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/mapping/Serializer.cpp.i"
	cd /home/tiger/CascadeProjects/oatpp-mariadb/tmp/build-debug/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/tiger/CascadeProjects/oatpp-mariadb/src/oatpp-mariadb/mapping/Serializer.cpp > CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/mapping/Serializer.cpp.i

src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/mapping/Serializer.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/mapping/Serializer.cpp.s"
	cd /home/tiger/CascadeProjects/oatpp-mariadb/tmp/build-debug/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/tiger/CascadeProjects/oatpp-mariadb/src/oatpp-mariadb/mapping/Serializer.cpp -o CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/mapping/Serializer.cpp.s

src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/ql_template/Parser.cpp.o: src/CMakeFiles/oatpp-mariadb.dir/flags.make
src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/ql_template/Parser.cpp.o: /home/tiger/CascadeProjects/oatpp-mariadb/src/oatpp-mariadb/ql_template/Parser.cpp
src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/ql_template/Parser.cpp.o: src/CMakeFiles/oatpp-mariadb.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/tiger/CascadeProjects/oatpp-mariadb/tmp/build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/ql_template/Parser.cpp.o"
	cd /home/tiger/CascadeProjects/oatpp-mariadb/tmp/build-debug/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/ql_template/Parser.cpp.o -MF CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/ql_template/Parser.cpp.o.d -o CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/ql_template/Parser.cpp.o -c /home/tiger/CascadeProjects/oatpp-mariadb/src/oatpp-mariadb/ql_template/Parser.cpp

src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/ql_template/Parser.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/ql_template/Parser.cpp.i"
	cd /home/tiger/CascadeProjects/oatpp-mariadb/tmp/build-debug/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/tiger/CascadeProjects/oatpp-mariadb/src/oatpp-mariadb/ql_template/Parser.cpp > CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/ql_template/Parser.cpp.i

src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/ql_template/Parser.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/ql_template/Parser.cpp.s"
	cd /home/tiger/CascadeProjects/oatpp-mariadb/tmp/build-debug/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/tiger/CascadeProjects/oatpp-mariadb/src/oatpp-mariadb/ql_template/Parser.cpp -o CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/ql_template/Parser.cpp.s

src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/ql_template/TemplateValueProvider.cpp.o: src/CMakeFiles/oatpp-mariadb.dir/flags.make
src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/ql_template/TemplateValueProvider.cpp.o: /home/tiger/CascadeProjects/oatpp-mariadb/src/oatpp-mariadb/ql_template/TemplateValueProvider.cpp
src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/ql_template/TemplateValueProvider.cpp.o: src/CMakeFiles/oatpp-mariadb.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/tiger/CascadeProjects/oatpp-mariadb/tmp/build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/ql_template/TemplateValueProvider.cpp.o"
	cd /home/tiger/CascadeProjects/oatpp-mariadb/tmp/build-debug/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/ql_template/TemplateValueProvider.cpp.o -MF CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/ql_template/TemplateValueProvider.cpp.o.d -o CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/ql_template/TemplateValueProvider.cpp.o -c /home/tiger/CascadeProjects/oatpp-mariadb/src/oatpp-mariadb/ql_template/TemplateValueProvider.cpp

src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/ql_template/TemplateValueProvider.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/ql_template/TemplateValueProvider.cpp.i"
	cd /home/tiger/CascadeProjects/oatpp-mariadb/tmp/build-debug/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/tiger/CascadeProjects/oatpp-mariadb/src/oatpp-mariadb/ql_template/TemplateValueProvider.cpp > CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/ql_template/TemplateValueProvider.cpp.i

src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/ql_template/TemplateValueProvider.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/ql_template/TemplateValueProvider.cpp.s"
	cd /home/tiger/CascadeProjects/oatpp-mariadb/tmp/build-debug/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/tiger/CascadeProjects/oatpp-mariadb/src/oatpp-mariadb/ql_template/TemplateValueProvider.cpp -o CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/ql_template/TemplateValueProvider.cpp.s

src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/Connection.cpp.o: src/CMakeFiles/oatpp-mariadb.dir/flags.make
src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/Connection.cpp.o: /home/tiger/CascadeProjects/oatpp-mariadb/src/oatpp-mariadb/Connection.cpp
src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/Connection.cpp.o: src/CMakeFiles/oatpp-mariadb.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/tiger/CascadeProjects/oatpp-mariadb/tmp/build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/Connection.cpp.o"
	cd /home/tiger/CascadeProjects/oatpp-mariadb/tmp/build-debug/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/Connection.cpp.o -MF CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/Connection.cpp.o.d -o CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/Connection.cpp.o -c /home/tiger/CascadeProjects/oatpp-mariadb/src/oatpp-mariadb/Connection.cpp

src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/Connection.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/Connection.cpp.i"
	cd /home/tiger/CascadeProjects/oatpp-mariadb/tmp/build-debug/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/tiger/CascadeProjects/oatpp-mariadb/src/oatpp-mariadb/Connection.cpp > CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/Connection.cpp.i

src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/Connection.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/Connection.cpp.s"
	cd /home/tiger/CascadeProjects/oatpp-mariadb/tmp/build-debug/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/tiger/CascadeProjects/oatpp-mariadb/src/oatpp-mariadb/Connection.cpp -o CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/Connection.cpp.s

src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/ConnectionProvider.cpp.o: src/CMakeFiles/oatpp-mariadb.dir/flags.make
src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/ConnectionProvider.cpp.o: /home/tiger/CascadeProjects/oatpp-mariadb/src/oatpp-mariadb/ConnectionProvider.cpp
src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/ConnectionProvider.cpp.o: src/CMakeFiles/oatpp-mariadb.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/tiger/CascadeProjects/oatpp-mariadb/tmp/build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building CXX object src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/ConnectionProvider.cpp.o"
	cd /home/tiger/CascadeProjects/oatpp-mariadb/tmp/build-debug/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/ConnectionProvider.cpp.o -MF CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/ConnectionProvider.cpp.o.d -o CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/ConnectionProvider.cpp.o -c /home/tiger/CascadeProjects/oatpp-mariadb/src/oatpp-mariadb/ConnectionProvider.cpp

src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/ConnectionProvider.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/ConnectionProvider.cpp.i"
	cd /home/tiger/CascadeProjects/oatpp-mariadb/tmp/build-debug/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/tiger/CascadeProjects/oatpp-mariadb/src/oatpp-mariadb/ConnectionProvider.cpp > CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/ConnectionProvider.cpp.i

src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/ConnectionProvider.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/ConnectionProvider.cpp.s"
	cd /home/tiger/CascadeProjects/oatpp-mariadb/tmp/build-debug/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/tiger/CascadeProjects/oatpp-mariadb/src/oatpp-mariadb/ConnectionProvider.cpp -o CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/ConnectionProvider.cpp.s

src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/Executor.cpp.o: src/CMakeFiles/oatpp-mariadb.dir/flags.make
src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/Executor.cpp.o: /home/tiger/CascadeProjects/oatpp-mariadb/src/oatpp-mariadb/Executor.cpp
src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/Executor.cpp.o: src/CMakeFiles/oatpp-mariadb.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/tiger/CascadeProjects/oatpp-mariadb/tmp/build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Building CXX object src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/Executor.cpp.o"
	cd /home/tiger/CascadeProjects/oatpp-mariadb/tmp/build-debug/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/Executor.cpp.o -MF CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/Executor.cpp.o.d -o CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/Executor.cpp.o -c /home/tiger/CascadeProjects/oatpp-mariadb/src/oatpp-mariadb/Executor.cpp

src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/Executor.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/Executor.cpp.i"
	cd /home/tiger/CascadeProjects/oatpp-mariadb/tmp/build-debug/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/tiger/CascadeProjects/oatpp-mariadb/src/oatpp-mariadb/Executor.cpp > CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/Executor.cpp.i

src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/Executor.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/Executor.cpp.s"
	cd /home/tiger/CascadeProjects/oatpp-mariadb/tmp/build-debug/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/tiger/CascadeProjects/oatpp-mariadb/src/oatpp-mariadb/Executor.cpp -o CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/Executor.cpp.s

src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/QueryResult.cpp.o: src/CMakeFiles/oatpp-mariadb.dir/flags.make
src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/QueryResult.cpp.o: /home/tiger/CascadeProjects/oatpp-mariadb/src/oatpp-mariadb/QueryResult.cpp
src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/QueryResult.cpp.o: src/CMakeFiles/oatpp-mariadb.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/tiger/CascadeProjects/oatpp-mariadb/tmp/build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_9) "Building CXX object src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/QueryResult.cpp.o"
	cd /home/tiger/CascadeProjects/oatpp-mariadb/tmp/build-debug/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/QueryResult.cpp.o -MF CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/QueryResult.cpp.o.d -o CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/QueryResult.cpp.o -c /home/tiger/CascadeProjects/oatpp-mariadb/src/oatpp-mariadb/QueryResult.cpp

src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/QueryResult.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/QueryResult.cpp.i"
	cd /home/tiger/CascadeProjects/oatpp-mariadb/tmp/build-debug/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/tiger/CascadeProjects/oatpp-mariadb/src/oatpp-mariadb/QueryResult.cpp > CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/QueryResult.cpp.i

src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/QueryResult.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/QueryResult.cpp.s"
	cd /home/tiger/CascadeProjects/oatpp-mariadb/tmp/build-debug/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/tiger/CascadeProjects/oatpp-mariadb/src/oatpp-mariadb/QueryResult.cpp -o CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/QueryResult.cpp.s

# Object files for target oatpp-mariadb
oatpp__mariadb_OBJECTS = \
"CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/mapping/Deserializer.cpp.o" \
"CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/mapping/ResultMapper.cpp.o" \
"CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/mapping/Serializer.cpp.o" \
"CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/ql_template/Parser.cpp.o" \
"CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/ql_template/TemplateValueProvider.cpp.o" \
"CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/Connection.cpp.o" \
"CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/ConnectionProvider.cpp.o" \
"CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/Executor.cpp.o" \
"CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/QueryResult.cpp.o"

# External object files for target oatpp-mariadb
oatpp__mariadb_EXTERNAL_OBJECTS =

src/liboatpp-mariadb.so: src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/mapping/Deserializer.cpp.o
src/liboatpp-mariadb.so: src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/mapping/ResultMapper.cpp.o
src/liboatpp-mariadb.so: src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/mapping/Serializer.cpp.o
src/liboatpp-mariadb.so: src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/ql_template/Parser.cpp.o
src/liboatpp-mariadb.so: src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/ql_template/TemplateValueProvider.cpp.o
src/liboatpp-mariadb.so: src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/Connection.cpp.o
src/liboatpp-mariadb.so: src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/ConnectionProvider.cpp.o
src/liboatpp-mariadb.so: src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/Executor.cpp.o
src/liboatpp-mariadb.so: src/CMakeFiles/oatpp-mariadb.dir/oatpp-mariadb/QueryResult.cpp.o
src/liboatpp-mariadb.so: src/CMakeFiles/oatpp-mariadb.dir/build.make
src/liboatpp-mariadb.so: src/CMakeFiles/oatpp-mariadb.dir/compiler_depend.ts
src/liboatpp-mariadb.so: /usr/local/lib/liboatpp-test.so
src/liboatpp-mariadb.so: /usr/local/lib/liboatpp.so
src/liboatpp-mariadb.so: src/CMakeFiles/oatpp-mariadb.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/home/tiger/CascadeProjects/oatpp-mariadb/tmp/build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_10) "Linking CXX shared library liboatpp-mariadb.so"
	cd /home/tiger/CascadeProjects/oatpp-mariadb/tmp/build-debug/src && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/oatpp-mariadb.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/CMakeFiles/oatpp-mariadb.dir/build: src/liboatpp-mariadb.so
.PHONY : src/CMakeFiles/oatpp-mariadb.dir/build

src/CMakeFiles/oatpp-mariadb.dir/clean:
	cd /home/tiger/CascadeProjects/oatpp-mariadb/tmp/build-debug/src && $(CMAKE_COMMAND) -P CMakeFiles/oatpp-mariadb.dir/cmake_clean.cmake
.PHONY : src/CMakeFiles/oatpp-mariadb.dir/clean

src/CMakeFiles/oatpp-mariadb.dir/depend:
	cd /home/tiger/CascadeProjects/oatpp-mariadb/tmp/build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/tiger/CascadeProjects/oatpp-mariadb /home/tiger/CascadeProjects/oatpp-mariadb/src /home/tiger/CascadeProjects/oatpp-mariadb/tmp/build-debug /home/tiger/CascadeProjects/oatpp-mariadb/tmp/build-debug/src /home/tiger/CascadeProjects/oatpp-mariadb/tmp/build-debug/src/CMakeFiles/oatpp-mariadb.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : src/CMakeFiles/oatpp-mariadb.dir/depend

