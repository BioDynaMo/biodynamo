cmake_minimum_required(VERSION 3.2.0)

# With CMAKE >= 3.2 external projects support byproducts, this is needed
# to support Ninja as generator

project(BioDynaMo)

#set(CMAKE_VERBOSE_MAKEFILE on)

# Options. Turn on with 'cmake -Dmyvarname=ON'.
option(test "Build all tests." ON) # Makes boolean 'test' available.
if(APPLE)
   # valgrind currently broken on macOS 10.12
   option(valgrind "Enable valgrind tests and make build compatible with valgrind tool." OFF)
else()
   option(valgrind "Enable valgrind tests and make build compatible with valgrind tool." ON)
endif()
option(coverage "Enable test coverage report generation." OFF)

#--------------------- find packages -------------
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
find_package(OpenMP REQUIRED)
# find_package(VTune REQUIRED)
if (test)
  # must be before include(${ROOT_USE_FILE}) otherwise test compile fails
  find_package(Threads REQUIRED)
endif()
find_package(Git)
#find_package(ROOT REQUIRED COMPONENTS Eve Geom Gui)

find_package(ClangTools)
if ("$ENV{CMAKE_EXPORT_COMPILE_COMMANDS}" STREQUAL "1" OR CLANG_TIDY_FOUND)
  # Generate a Clang compile_commands.json "compilation database" file for use
  # with various development tools, such as Vim's YouCompleteMe plugin.
  # See http://clang.llvm.org/docs/JSONCompilationDatabase.html
  set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
endif()

#--------------------- includes -------------
include(ExternalProject)
include(cmake/doxygen.cmake)

#--------------------- check if compiler supports C++11 -------------
# http://stackoverflow.com/questions/10851247/how-to-activate-c-11-in-cmake
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-std=c++14" COMPILER_SUPPORTS_CXX14)
if(COMPILER_SUPPORTS_CXX14)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
else()
  message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++14 support. Please use a different C++ compiler.")
endif()

#--------------------- set compiler flags -------------
if (coverage)
  set(CMAKE_BUILD_TYPE "Debug")
  set(GCC_COMPILE_FLAGS "-Wall -Wno-missing-braces -g -O0 -fprofile-arcs -ftest-coverage")
else()
  set(GCC_COMPILE_FLAGS "-Wall -Wno-missing-braces")
endif()
set(GCC_LINK_FLAGS "")
# append
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GCC_COMPILE_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COMPILE_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GCC_LINK_FLAGS}")
# append OpenMP flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
# special clang flag
if(${CMAKE_CXX_COMPILER_ID} MATCHES Clang)
  # silence unsupported -pthread argument at link time
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Qunused-arguments")
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Qunused-arguments")
  # silence clang 3.9 warning
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-undefined-var-template")
endif()

#--------------------- set rpath options -------------
# When building, use the RPATH
set(CMAKE_SKIP_BUILD_RPATH FALSE)           # don't skip the full RPATH for the build tree
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)   # use always the build RPATH for the build tree
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE) # point to directories outside the build tree to the install RPATH

if(APPLE)
  set(CMAKE_MACOSX_RPATH TRUE)                 # use RPATH for MacOSX
  set(CMAKE_INSTALL_NAME_DIR "@rpath")
  set(CMAKE_INSTALL_RPATH "@loader_path/")     # self relative LIBDIR
else()
  set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_FULL_LIBDIR}) # install LIBDIR
endif()

#--------------------- setup Vc ----------------
# setup Vc
# ExternalProject_Add(
#     Vc
#     URL "${CMAKE_SOURCE_DIR}/third_party/Vc-master_2bfce51.zip"
#     PREFIX "${CMAKE_CURRENT_BINARY_DIR}/Vc"
#     CMAKE_ARGS "-DBUILD_TESTING=off"
#     CMAKE_CACHE_ARGS
#     -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
#     -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
#     INSTALL_COMMAND "" # Disable install step
#     # Ugly but necessary, in future versions one can use ${binary_dir}
#     # in BUILD_BYPRODUCTS
#     #BUILD_BYPRODUCTS "${binary_dir}/libVc.a"
#     BUILD_BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/Vc/src/Vc-build/libVc.a"
# )
# set(Vc_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/Vc/src/Vc/include"
#                    "${CMAKE_CURRENT_BINARY_DIR}/Vc/src/Vc/")
# set(Vc_LIBRARIES "${CMAKE_CURRENT_BINARY_DIR}/Vc/src/Vc-build/libVc.a")

#--------------------- setup mpark variant library ----------------
set(MPARK_LIB "${CMAKE_CURRENT_BINARY_DIR}/mpark/")
file(MAKE_DIRECTORY "${MPARK_LIB}/")
execute_process(COMMAND ${CMAKE_COMMAND} -E tar xf "${CMAKE_SOURCE_DIR}/third_party/mpark_4ee0086.zip"
                WORKING_DIRECTORY "${MPARK_LIB}")

#--------------------- build shared library libbiodynamo ----------------
# add the headers into the project
#include(${ROOT_USE_FILE})
include_directories("${CMAKE_SOURCE_DIR}/src")
include_directories("${CMAKE_SOURCE_DIR}/third_party")
include_directories("${MPARK_LIB}")
# include_directories("${Vc_INCLUDE_DIR}")
# include_directories(${VTUNE_INCLUDE_DIR})

#ROOT_GENERATE_DICTIONARY("${CMAKE_BINARY_DIR}/bdmDict"
#    simulation/ecm.h
#    LINKDEF bdmLinkDef.h)

file(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/src/*.cc")
file(GLOB_RECURSE HEADERS "${CMAKE_SOURCE_DIR}/src/*.h")
#${CMAKE_BINARY_DIR}/bdmDict.cxx)
add_library(biodynamo SHARED ${SOURCES} ${HEADERS})
#target_link_libraries(biodynamo ${ROOT_LIBRARIES})
#target_link_libraries(biodynamo ${Vc_LIBRARIES})
# target_link_libraries(biodynamo ${VTUNE_LIBRARIES})
# add_dependencies(biodynamo Vc)

# add custom clean target for shared library
add_custom_target(libbdmclean COMMAND ${CMAKE_COMMAND} -P "${CMAKE_BINARY_DIR}/CMakeFiles/biodynamo.dir/cmake_clean.cmake")
# custom clean target for biodynamo that excludes ExternalProjects
add_custom_target(bdmclean)
add_dependencies(bdmclean libbdmclean)

# -------------------- build test project  ----------------------
if (test)
  enable_testing()

  include(cmake/Testing.cmake)

  # main test executable
  file(GLOB TEST_SOURCES ${CMAKE_SOURCE_DIR}/test/*.cc
                         ${CMAKE_SOURCE_DIR}/test/spatial_organization/*.cc)
  create_test_executable(runBiodynamoTestsMain "${TEST_SOURCES}")

  # separate test binaries - each source file is compiled into a separate binary
  file(GLOB TEST_SOURCES ${CMAKE_SOURCE_DIR}/test/separate_binary/*.cc)
  foreach(file ${TEST_SOURCES})
    get_filename_component(filename ${file} NAME_WE)
    create_test_executable("runBiodynamoTests_${filename}" ${file})
  endforeach()
endif()

# --------------- add targets for code style and submission checks -------------
include(cmake/CppStyleGuideChecks.cmake)

# --------------------- build cell_growth executable ---------------------
set(SIMULATION_TARGET cell_growth)
add_executable(${SIMULATION_TARGET} ${CMAKE_SOURCE_DIR}/demo/cell_growth.cc)
target_link_libraries(${SIMULATION_TARGET} biodynamo)

# --------------------- build cell_division_module executable ------------------
set(SIMULATION_TARGET cell_division_module)
add_executable(${SIMULATION_TARGET} ${CMAKE_SOURCE_DIR}/demo/cell_division_module.cc)
target_link_libraries(${SIMULATION_TARGET} biodynamo)
