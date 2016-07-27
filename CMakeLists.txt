cmake_minimum_required(VERSION 2.8.9)

project(BioDynaMo)

#set(CMAKE_VERBOSE_MAKEFILE on)

# Options. Turn on with 'cmake -Dmyvarname=ON'.
option(test "Build all tests." ON) # Makes boolean 'test' available.
option(valgrind "Enable valgrind tests and make build compatible with valgrind tool." ON)

#--------------------- set compiler flags -------------
set(GCC_COMPILE_FLAGS "-Wall -Wno-missing-braces -O2")
set(GCC_LINK_FLAGS    "")
# append
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GCC_COMPILE_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COMPILE_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GCC_LINK_FLAGS}")

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

#--------------------- setup GMP ------------------------------
# Enable ExternalProject CMake module
include(ExternalProject)

if (valgrind)
  set(GMP_CONFIG_PARAMETER "--disable-assembly")
else()
  set(GMP_CONFIG_PARAMETER "")
endif()

# download gmp
ExternalProject_Add(
        gmp
        URL "${CMAKE_SOURCE_DIR}/external/gmp-6.1.0.tar.bz2"
        PREFIX "${CMAKE_CURRENT_BINARY_DIR}/gmp"
        BUILD_IN_SOURCE 1
        CONFIGURE_COMMAND ./configure CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER} ${GMP_CONFIG_PARAMETER}
        BUILD_COMMAND make
        #TEST_COMMAND make check
        TEST_COMMAND "" # Disable check step
        INSTALL_COMMAND "" # Disable install step
)

if(APPLE)
  ExternalProject_Add_Step(gmp install_name
          COMMAND install_name_tool -id @rpath/libgmp.10.dylib "${CMAKE_BINARY_DIR}/gmp/src/gmp/.libs/libgmp${CMAKE_SHARED_LIBRARY_SUFFIX}"
          DEPENDEES install)
endif()

# Create a libgmp target to be used as a dependency by the test program
add_library(libgmp IMPORTED STATIC GLOBAL)
set_property(TARGET libgmp PROPERTY IMPORTED_LOCATION "${CMAKE_BINARY_DIR}/gmp/src/gmp/.libs/libgmp${CMAKE_SHARED_LIBRARY_SUFFIX}")
add_dependencies(libgmp gmp)

# copy shared library from gmp build dir to ${CMAKE_BUILD_DIR}
if(APPLE)
  set(LIBGMP_DEST "${CMAKE_BINARY_DIR}/libgmp.10${CMAKE_SHARED_LIBRARY_SUFFIX}")
elseif(UNIX)
  set(LIBGMP_DEST "${CMAKE_BINARY_DIR}/libgmp${CMAKE_SHARED_LIBRARY_SUFFIX}.10")
endif()
add_custom_target(copylibgmp DEPENDS libgmp
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_BINARY_DIR}/gmp/src/gmp/.libs/libgmp${CMAKE_SHARED_LIBRARY_SUFFIX}" ${LIBGMP_DEST})

#--------------------- setup crlibm ------------------------------
# build crlibm
ExternalProject_Add(
        external_crlibm
        URL "${CMAKE_SOURCE_DIR}/external/crlibm-1.0beta5.tgz"
        PREFIX "${CMAKE_CURRENT_BINARY_DIR}/crlibm"
        BUILD_IN_SOURCE 1
        CONFIGURE_COMMAND ./configure CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER} CPPFLAGS=-fPIC CFLAGS=-fPIC
        BUILD_COMMAND make
        #TEST_COMMAND make check
        TEST_COMMAND "" # Disable check step
        INSTALL_COMMAND "" # Disable install step
)

add_custom_target(copycrlibm DEPENDS external_crlibm
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_BINARY_DIR}/crlibm/src/external_crlibm/libcrlibm.a" "${CMAKE_BINARY_DIR}/libcrlibm.a")

#--------------------- Find Threads (for gtest) ------------------
if (test)
  # must be before include(${ROOT_USE_FILE}) otherwise test compile fails
  find_package(Threads REQUIRED)
endif()

#--------------------- Find ROOT ---------------------------------
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
find_package(ROOT REQUIRED COMPONENTS Eve Geom Gui)
include(${ROOT_USE_FILE})

#--------------------- Find Doxygen and set doc target -----------
include(cmake/doxygen.cmake)

#--------------------- build shared library  ----------------
# add the headers into the project
include_directories("${CMAKE_SOURCE_DIR}/include")
include_directories("${CMAKE_BINARY_DIR}/gmp/src/gmp")
include_directories("${CMAKE_BINARY_DIR}/crlibm/src/external_crlibm")

ROOT_GENERATE_DICTIONARY("${CMAKE_BINARY_DIR}/bdmDict"
        simulation/ecm.h
        LINKDEF bdmLinkDef.h)

file(GLOB SOURCES "src/**/*.cc" "src/*.cc" "include/**/*.h" "include/*.h")
add_library(biodynamo SHARED ${SOURCES} ${CMAKE_BINARY_DIR}/bdmDict.cxx)
target_link_libraries(biodynamo ${ROOT_LIBRARIES})
target_link_libraries(biodynamo libgmp)
add_dependencies(biodynamo copylibgmp)
target_link_libraries(biodynamo "${CMAKE_BINARY_DIR}/libcrlibm.a")
add_dependencies(biodynamo copycrlibm)

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
  ExternalProject_Add(
          gtest
          URL "${CMAKE_SOURCE_DIR}/external/gtest-1.7.0.zip"
          PREFIX "${CMAKE_CURRENT_BINARY_DIR}/gtest"
          CMAKE_CACHE_ARGS
          -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
          -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
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
          URL "${CMAKE_SOURCE_DIR}/external/jsoncpp.tar.gz"
          PREFIX "${CMAKE_CURRENT_BINARY_DIR}/jsoncpp"
          CMAKE_CACHE_ARGS
          -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
          -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
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
  include_directories(${ROOT_INCLUDE_DIRS})

  # create test executable
  file(GLOB TEST_SOURCES "test/*.cc" "test/*.h")
  set(TEST_TARGET runBiodynamoTests)
  add_executable(${TEST_TARGET} ${TEST_SOURCES})
  target_link_libraries(${TEST_TARGET} libgtest)
  target_link_libraries(${TEST_TARGET} libjsoncpp)
  target_link_libraries(${TEST_TARGET} biodynamo)
  target_link_libraries(${TEST_TARGET} ${ROOT_LIBRARIES})

  # copy test resources to build dir
  file(GLOB TOCOPY "test/resources/*")
  file(COPY ${TOCOPY} DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/test_resources/")

  # execute all tests with command: make test
  add_test(NAME ${TEST_TARGET} COMMAND ${TEST_TARGET})
  # add valgrind test - exclude Figure9Test because it would take too long
  if (valgrind)
    add_test(NAME valgrind COMMAND valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --suppressions=${CMAKE_CURRENT_BINARY_DIR}/../valgrind-biod.supp --error-exitcode=1 ./${TEST_TARGET} --disable-assertions --gtest_filter=-DividingModule*:SomaClustering*:Figure9*:Intracellular*:SmallNetwork*)
  endif()

  # create target that shows the test ouput on failure
  add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND}  --force-new-ctest-process --output-on-failure)

  # add custom clean target for test project
  add_custom_target(testbdmclean COMMAND ${CMAKE_COMMAND} -P "${CMAKE_BINARY_DIR}/CMakeFiles/${TEST_TARGET}.dir/cmake_clean.cmake")
  add_dependencies(bdmclean testbdmclean)
endif()

#--------------------- build visualization projects ----------------------
# build viz executable for small network test
set(SMALL_NW_VIZ small-network-viz)
add_executable(${SMALL_NW_VIZ} ${SOURCES} viz/small_network_visualization.cpp)
target_include_directories(${SMALL_NW_VIZ} PUBLIC include)
target_link_libraries(${SMALL_NW_VIZ} biodynamo)
target_link_libraries(${SMALL_NW_VIZ} ${ROOT_LIBRARIES})

# build viz executable for dividing cell test
set(DIVIDING_CELL_VIZ dividing-cell-viz)
add_executable(${DIVIDING_CELL_VIZ} ${SOURCES} viz/dividing-cell-viz.cpp)
target_include_directories(${DIVIDING_CELL_VIZ} PUBLIC include)
target_link_libraries(${DIVIDING_CELL_VIZ} biodynamo)
target_link_libraries(${DIVIDING_CELL_VIZ} ${ROOT_LIBRARIES})