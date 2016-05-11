cmake_minimum_required(VERSION 2.8.9)

project(biodynamo)
set(CMAKE_BUILD_TYPE Debug)

# Options. Turn on with 'cmake -Dmyvarname=ON'.
option(test "Build all tests." ON) # Makes boolean 'test' available.

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
# Enable ExternalProject CMake module
include(ExternalProject)

# download gmp
ExternalProject_Add(
    gmp
    URL https://gmplib.org/download/gmp/gmp-6.1.0.tar.bz2
    PREFIX "${CMAKE_CURRENT_BINARY_DIR}/gmp"
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND "./configure"
    BUILD_COMMAND make
    TEST_COMMAND make check
    INSTALL_COMMAND "" # Disable install step
)

# Create a libjsoncpp target to be used as a dependency by the test program
add_library(libgmp IMPORTED STATIC GLOBAL)
set_property(TARGET libgmp PROPERTY IMPORTED_LOCATION "${CMAKE_BINARY_DIR}/gmp/src/gmp/.libs/libgmp${CMAKE_SHARED_LIBRARY_SUFFIX}")
add_dependencies(libgmp gmp)

# copy shared library from gmp build dir to ${CMAKE_BUILD_DIR}
if(APPLE)
  set(LIBGMP_DEST "${CMAKE_BINARY_DIR}/libgmp.10${CMAKE_SHARED_LIBRARY_SUFFIX}")
elseif(UNIX)
  set(LIBGMP_DEST "${CMAKE_BINARY_DIR}/libgmp${CMAKE_SHARED_LIBRARY_SUFFIX}.10")
endif()
add_custom_target(copylibgmp DEPENDS libgmp COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_BINARY_DIR}/gmp/src/gmp/.libs/libgmp${CMAKE_SHARED_LIBRARY_SUFFIX}" ${LIBGMP_DEST})

#--------------------- Doxygen ---------------------------------
add_subdirectory(doc doc)

#--------------------- build shared library  ----------------
# add the headers into the project
include_directories(include)
include_directories("${CMAKE_BINARY_DIR}/gmp/src/gmp")

file(GLOB SOURCES "src/**/*.cc" "src/*.cc" "include/**/*.h" "include/*.h")
add_library(biodynamo SHARED ${SOURCES})
target_link_libraries(biodynamo libgmp)
add_dependencies(biodynamo copylibgmp)

# add custom clean target for shared library
add_custom_target(libbdmclean COMMAND ${CMAKE_COMMAND} -P "${CMAKE_BINARY_DIR}/CMakeFiles/biodynamo.dir/cmake_clean.cmake")
# custom clean target for biodynamo that excludes ExternalProjects
add_custom_target(bdmclean)
add_dependencies(bdmclean libbdmclean)

#--------------------- build test project ----------------------
# http://www.kaizou.org/2014/11/gtest-cmake/
if (test)
  enable_testing()

  # setup google test
  find_package(Threads REQUIRED)
  ExternalProject_Add(
    gtest
    URL https://googletest.googlecode.com/files/gtest-1.7.0.zip
    PREFIX "${CMAKE_CURRENT_BINARY_DIR}/gtest"
    INSTALL_COMMAND "" # Disable install step
  )
  ExternalProject_Get_Property(gtest source_dir binary_dir)

  # Create a libgtest target to be used as a dependency by test program
  add_library(libgtest IMPORTED STATIC GLOBAL)
  add_dependencies(libgtest gtest)
  set_target_properties(libgtest PROPERTIES
      IMPORTED_LOCATION "${binary_dir}/libgtest.a"
      IMPORTED_LINK_INTERFACE_LIBRARIES "${CMAKE_THREAD_LIBS_INIT}"
  )

  # setup jsoncpp
  ExternalProject_Add(
        jsoncpp
        URL https://github.com/open-source-parsers/jsoncpp/archive/1.7.2.zip
        PREFIX "${CMAKE_CURRENT_BINARY_DIR}/jsoncpp"
        INSTALL_COMMAND ""  # Disable install step
  )
  ExternalProject_Get_Property(jsoncpp install_dir)

  # Create a libjsoncpp target to be used as a dependency by test program
  add_library(libjsoncpp IMPORTED STATIC GLOBAL)
  set_property(TARGET libjsoncpp PROPERTY IMPORTED_LOCATION "${install_dir}/src/jsoncpp-build/src/lib_json/libjsoncpp.a")
  add_dependencies(libjsoncpp jsoncpp)

  # add include directories for gtest and jsoncpp
  include_directories("${CMAKE_BINARY_DIR}/gtest/src/gtest/include")
  include_directories("${CMAKE_BINARY_DIR}/jsoncpp/src/jsoncpp/include")

  # create test executable
  file(GLOB TEST_SOURCES "test/*.cc" "test/*.h")
  set(TEST_TARGET runBiodynamoTests)
  add_executable(${TEST_TARGET} ${TEST_SOURCES})
  target_link_libraries(${TEST_TARGET} libgtest)
  target_link_libraries(${TEST_TARGET} libjsoncpp)
  target_link_libraries(${TEST_TARGET} biodynamo)

  # copy test resources to build dir
  file(GLOB TOCOPY "test/resources/*")
  file(COPY ${TOCOPY} DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/test_resources/")

  # execute all tests with command: make test
  add_test(NAME ${TEST_TARGET} COMMAND ${TEST_TARGET})

  # add custom clean target for test project
  add_custom_target(testbdmclean COMMAND ${CMAKE_COMMAND} -P "${CMAKE_BINARY_DIR}/CMakeFiles/${TEST_TARGET}.dir/cmake_clean.cmake")
  add_dependencies(bdmclean testbdmclean)
endif()

