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
find_package(ROOT REQUIRED COMPONENTS Eve Geom Gui)

find_package(ClangTools)
if ("$ENV{CMAKE_EXPORT_COMPILE_COMMANDS}" STREQUAL "1" OR CLANG_TIDY_FOUND)
  # Generate a Clang compile_commands.json "compilation database" file for use
  # with various development tools, such as Vim's YouCompleteMe plugin.
  # See http://clang.llvm.org/docs/JSONCompilationDatabase.html
  set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
endif()

#--------------------- includes -------------
include(BioDynaMo)
include(ExternalProject)
include(cmake/doxygen.cmake)

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
include(${ROOT_USE_FILE})
include_directories("${CMAKE_SOURCE_DIR}/src")
include_directories("${CMAKE_SOURCE_DIR}/test")
include_directories("${CMAKE_SOURCE_DIR}/third_party")
include_directories("${CMAKE_SOURCE_DIR}/demo")
include_directories("${MPARK_LIB}")
# include_directories("${Vc_INCLUDE_DIR}")
# include_directories(${VTUNE_INCLUDE_DIR})

# build libbiodynamo
file(GLOB_RECURSE HEADERS "${CMAKE_SOURCE_DIR}/src/*.h")
set(LIBBDM_DICT_WE "libbiodynamo_dict")
REFLEX_GENERATE_DICTIONARY("${LIBBDM_DICT_WE}"
                           ${HEADERS}
                           SELECTION cmake/selection-libbiodynamo.xml)
file(GLOB_RECURSE LIB_SOURCES "${CMAKE_SOURCE_DIR}/src/*.cc")
add_library(biodynamo SHARED ${LIB_SOURCES} ${LIBBDM_DICT_WE}.cc)
target_link_libraries(biodynamo ${ROOT_LIBRARIES})
# target_link_libraries(biodynamo ${Vc_LIBRARIES})
# target_link_libraries(biodynamo ${VTUNE_LIBRARIES})
# add_dependencies(biodynamo Vc)

# add custom clean target for shared library
add_custom_target(libbdmclean COMMAND ${CMAKE_COMMAND} -P "${CMAKE_BINARY_DIR}/CMakeFiles/biodynamo.dir/cmake_clean.cmake")
# custom clean target for biodynamo that excludes ExternalProjects
add_custom_target(bdmclean)
add_dependencies(bdmclean libbdmclean)

generate_rootlogon()

# -------------------- build test project runBiodynamoTests ----------------------
# http://www.kaizou.org/2014/11/gtest-cmake/
if (test)
  enable_testing()

  # setup google test
  ExternalProject_Add(
    gtest
    URL "${CMAKE_SOURCE_DIR}/third_party/gtest-1.7.0.zip"
    PREFIX "${CMAKE_CURRENT_BINARY_DIR}/gtest"
    CMAKE_CACHE_ARGS
      -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
      -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
    INSTALL_COMMAND "" # Disable install step
    # Ugly but necessary, in future versions one can use ${binary_dir}
    # in BUILD_BYPRODUCTS
    #BUILD_BYPRODUCTS "${binary_dir}/libgtest.a"
    BUILD_BYPRODUCTS "${CMAKE_BINARY_DIR}/gtest/src/gtest-build/libgtest.a"
  )
  ExternalProject_Get_Property(gtest source_dir binary_dir)

  # Create a libgtest target to be used as a dependency by test program
  add_library(libgtest IMPORTED STATIC GLOBAL)
  add_dependencies(libgtest gtest)
  set_target_properties(libgtest PROPERTIES
      IMPORTED_LOCATION "${binary_dir}/libgtest.a"
      IMPORTED_LINK_INTERFACE_LIBRARIES "${CMAKE_THREAD_LIBS_INIT}"
  )

  # add include directories for gtest
  include_directories("${CMAKE_BINARY_DIR}/gtest/src/gtest/include")

  # create test executable
  file(GLOB_RECURSE TEST_SOURCES ${CMAKE_SOURCE_DIR}/test/*.cc)
  set(TEST_TARGET runBiodynamoTests)
  bdm_add_executable(${TEST_TARGET}
                     SOURCES ${TEST_SOURCES}
                     LIBRARIES libgtest biodynamo)

  # execute all tests with command: make test
  add_test(NAME ${TEST_TARGET} COMMAND ${TEST_TARGET})
  if (APPLE)
    set_tests_properties(${TEST_TARGET} PROPERTIES ENVIRONMENT
        "DYLD_LIBRARY_PATH=$ENV{DYLD_LIBRARY_PATH}"
    )
  endif()
  # add valgrind test
  if (valgrind)
    add_test(NAME valgrind COMMAND valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --show-reachable=no --suppressions=${CMAKE_BINARY_DIR}/../valgrind-root.supp --suppressions=${CMAKE_BINARY_DIR}/../valgrind-biod.supp --error-exitcode=1 ./${TEST_TARGET})
    if (APPLE)
      set_tests_properties(valgrind PROPERTIES ENVIRONMENT
        "DYLD_LIBRARY_PATH=$ENV{DYLD_LIBRARY_PATH}"
      )
    endif()
  endif()

  # create target that shows the test ouput on failure
  add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} --force-new-ctest-process --output-on-failure)
  add_dependencies(check runBiodynamoTests)

  # add custom clean target for test project
  add_custom_target(testbdmclean COMMAND ${CMAKE_COMMAND} -P "${CMAKE_BINARY_DIR}/CMakeFiles/${TEST_TARGET}.dir/cmake_clean.cmake")
  add_dependencies(bdmclean testbdmclean)

  if (coverage)
    include(CodeCoverage)
    # parameter: make target name, test target, output directory name
    SETUP_TARGET_FOR_COVERAGE(coverage ${TEST_TARGET} coverage)
  endif()
  # create coverage report in separate directory
  # since building the coverage report requires different compiler flags building
  # it in a separate directory keeps the current build directory in good order
  add_custom_target(coverage-build
    COMMAND "${CMAKE_SOURCE_DIR}/housekeeping/create-coverage-report.sh" ${PROJECT_SOURCE_DIR} ${CMAKE_BINARY_DIR}
    COMMENT "Generate coverage report in separate directory
       Open the following file in your browser: ${CMAKE_BINARY_DIR}/coverage/coverage/index.html")
endif()

# --------------- add targets for code style and submission checks -------------
include(cmake/CppStyleGuideChecks.cmake)

# --------------------- build cell_growth executable ---------------------
bdm_add_executable(cell_growth
                   SOURCES demo/cell_growth.cc
                   LIBRARIES biodynamo)

# --------------------- build cell_division_module executable ------------------
bdm_add_executable(cell_division_module
                   SOURCES demo/cell_division_module.cc
                   LIBRARIES biodynamo)

# --------------------- build persistent simulation demo ---------------------
bdm_add_executable(persistent_sim
                   SOURCES demo/persistent_sim.cc
                   LIBRARIES biodynamo)
