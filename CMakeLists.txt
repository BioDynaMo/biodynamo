cmake_minimum_required(VERSION 2.8.9)

# Options. Turn on with 'cmake -Dmyvarname=ON'.
option(test "Build all tests." OFF) # Makes boolean 'test' available.

project(biodynamo)
set(CMAKE_BUILD_TYPE Debug)

# include custom cmake files (requires absolute path)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

#--------------------- set compiler flags -------------
set(GCC_COMPILE_FLAGS "-w -O2")
set(GCC_LINK_FLAGS    "")
# append
set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COMPILE_FLAGS}" )
set( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_LINK_FLAGS}")

#--------------------- check if compiler supports C++11 -------------
# http://stackoverflow.com/questions/10851247/how-to-activate-c-11-in-cmake
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-std=c++11" COMPILER_SUPPORTS_CXX11)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

#--------------------- setup GMP ------------------------------
find_package(GMP REQUIRED)
include_directories(${GMP_INCLUDE_DIR})

#--------------------- Doxygen ---------------------------------
add_subdirectory(doc doc)

#---------------------------------------------------------------
#--------------------- build main project ----------------------
#---------------------------------------------------------------

#--------------------- add source files to project -------------
# add the headers into the project
include_directories(include)

#--------------------- build shared libray  ----------------
file(GLOB SOURCES "src/**/*.cc" "src/*.cc" "**/*.h")
add_library(biodynamo SHARED ${SOURCES})
target_link_libraries(biodynamo ${GMP_LIBRARY})

#---------------------------------------------------------------
#--------------------- build test project ----------------------
#---------------------------------------------------------------
# http://www.kaizou.org/2014/11/gtest-cmake/
if (test)
  enable_testing()
    # We need thread support
  find_package(Threads REQUIRED)

  # Enable ExternalProject CMake module
  include(ExternalProject)

  # Download and install GoogleTest
  ExternalProject_Add(
      gtest
      URL https://googletest.googlecode.com/files/gtest-1.7.0.zip
      PREFIX ${CMAKE_CURRENT_BINARY_DIR}/gtest
      # Disable install step
      INSTALL_COMMAND ""
  )

  # Create a libgtest target to be used as a dependency by test programs
  add_library(libgtest IMPORTED STATIC GLOBAL)
  add_dependencies(libgtest gtest)

  # Set gtest properties
  ExternalProject_Get_Property(gtest source_dir binary_dir)
  set_target_properties(libgtest PROPERTIES
      "IMPORTED_LOCATION" "${binary_dir}/libgtest.a"
      "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
  #    "INTERFACE_INCLUDE_DIRECTORIES" "${source_dir}/include"
  )
  # INTERFACE_INCLUDE_DIRECTORIES didn't work
  include_directories("${source_dir}/include")

  # create test executable
  add_executable(testBiodynamo "test/simulation_tests.cc" "test/main.cc")
  target_link_libraries(testBiodynamo  libgtest)
  target_link_libraries(testBiodynamo biodynamo)

  add_test(NAME testBiodynamo COMMAND testBiodynamo)
endif()
