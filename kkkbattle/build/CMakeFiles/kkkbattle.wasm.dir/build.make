# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
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
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/gyl/eosio.cdt/contracts/kkkbattle

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/gyl/eosio.cdt/contracts/kkkbattle/build

# Include any dependencies generated for this target.
include CMakeFiles/kkkbattle.wasm.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/kkkbattle.wasm.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/kkkbattle.wasm.dir/flags.make

CMakeFiles/kkkbattle.wasm.dir/kkkbattle.cpp.o: CMakeFiles/kkkbattle.wasm.dir/flags.make
CMakeFiles/kkkbattle.wasm.dir/kkkbattle.cpp.o: ../kkkbattle.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/gyl/eosio.cdt/contracts/kkkbattle/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/kkkbattle.wasm.dir/kkkbattle.cpp.o"
	/usr/local/eosio.cdt/bin/eosio-cpp  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/kkkbattle.wasm.dir/kkkbattle.cpp.o -c /home/gyl/eosio.cdt/contracts/kkkbattle/kkkbattle.cpp

CMakeFiles/kkkbattle.wasm.dir/kkkbattle.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/kkkbattle.wasm.dir/kkkbattle.cpp.i"
	/usr/local/eosio.cdt/bin/eosio-cpp $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/gyl/eosio.cdt/contracts/kkkbattle/kkkbattle.cpp > CMakeFiles/kkkbattle.wasm.dir/kkkbattle.cpp.i

CMakeFiles/kkkbattle.wasm.dir/kkkbattle.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/kkkbattle.wasm.dir/kkkbattle.cpp.s"
	/usr/local/eosio.cdt/bin/eosio-cpp $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/gyl/eosio.cdt/contracts/kkkbattle/kkkbattle.cpp -o CMakeFiles/kkkbattle.wasm.dir/kkkbattle.cpp.s

CMakeFiles/kkkbattle.wasm.dir/kkkbattle.cpp.o.requires:

.PHONY : CMakeFiles/kkkbattle.wasm.dir/kkkbattle.cpp.o.requires

CMakeFiles/kkkbattle.wasm.dir/kkkbattle.cpp.o.provides: CMakeFiles/kkkbattle.wasm.dir/kkkbattle.cpp.o.requires
	$(MAKE) -f CMakeFiles/kkkbattle.wasm.dir/build.make CMakeFiles/kkkbattle.wasm.dir/kkkbattle.cpp.o.provides.build
.PHONY : CMakeFiles/kkkbattle.wasm.dir/kkkbattle.cpp.o.provides

CMakeFiles/kkkbattle.wasm.dir/kkkbattle.cpp.o.provides.build: CMakeFiles/kkkbattle.wasm.dir/kkkbattle.cpp.o


# Object files for target kkkbattle.wasm
kkkbattle_wasm_OBJECTS = \
"CMakeFiles/kkkbattle.wasm.dir/kkkbattle.cpp.o"

# External object files for target kkkbattle.wasm
kkkbattle_wasm_EXTERNAL_OBJECTS =

kkkbattle.wasm: CMakeFiles/kkkbattle.wasm.dir/kkkbattle.cpp.o
kkkbattle.wasm: CMakeFiles/kkkbattle.wasm.dir/build.make
kkkbattle.wasm: CMakeFiles/kkkbattle.wasm.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/gyl/eosio.cdt/contracts/kkkbattle/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable kkkbattle.wasm"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/kkkbattle.wasm.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/kkkbattle.wasm.dir/build: kkkbattle.wasm

.PHONY : CMakeFiles/kkkbattle.wasm.dir/build

CMakeFiles/kkkbattle.wasm.dir/requires: CMakeFiles/kkkbattle.wasm.dir/kkkbattle.cpp.o.requires

.PHONY : CMakeFiles/kkkbattle.wasm.dir/requires

CMakeFiles/kkkbattle.wasm.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/kkkbattle.wasm.dir/cmake_clean.cmake
.PHONY : CMakeFiles/kkkbattle.wasm.dir/clean

CMakeFiles/kkkbattle.wasm.dir/depend:
	cd /home/gyl/eosio.cdt/contracts/kkkbattle/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/gyl/eosio.cdt/contracts/kkkbattle /home/gyl/eosio.cdt/contracts/kkkbattle /home/gyl/eosio.cdt/contracts/kkkbattle/build /home/gyl/eosio.cdt/contracts/kkkbattle/build /home/gyl/eosio.cdt/contracts/kkkbattle/build/CMakeFiles/kkkbattle.wasm.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/kkkbattle.wasm.dir/depend

