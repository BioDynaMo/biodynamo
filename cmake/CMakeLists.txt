cmake_minimum_required(VERSION 2.8.9)
project(cx3d-cpp)
#set(CMAKE_BUILD_TYPE Release)

# include custom cmake files
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "modules/")

#--------------------- path definitions ----------------------
#main
set(CPP_MAIN_SRC_PATH ../src/main/cpp)
set(JAVA_MAIN_RESOURCE_PATH ../src/main/resources)
set(JAVA_MAIN_SWIG_PATH ../src/main/java/ini/cx3d/swig)
#test
set(CPP_TEST_SRC_PATH ../src/test/cpp)
set(JAVA_TEST_RESOURCE_PATH ../src/test/resources)
set(JAVA_TEST_SWIG_PATH ../src/test/java/ini/cx3d/swig)

#--------------------- set compiler flags -------------
set(GCC_COMPILE_FLAGS "-Wall -Wextra")
set(GCC_LINK_FLAGS    "")
# append
set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COMPILE_FLAGS}" )
set( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_LINK_FLAGS}")

#--------------------- check if compiler supports C++11 -------------
#http://stackoverflow.com/questions/10851247/how-to-activate-c-11-in-cmake
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-std=c++11" COMPILER_SUPPORTS_CXX11)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

#--------------------- setup swig ------------------------------
#swig Java wrappers
find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})

find_package(JNI REQUIRED)
include_directories(${JAVA_INCLUDE_PATH})       #path to jni.h
include_directories(${JAVA_INCLUDE_PATH2})      #path to jni_md.h
include_directories(${SWIG_DIR}/java)           #path to swig java includes

get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY include_directories)
foreach(dir ${dirs})
  message(STATUS "dir='${dir}'")
endforeach()

#--------------------- setup GMP ------------------------------
find_package(GMP REQUIRED)
include_directories(${GMP_INCLUDE_DIR})

#--------------------- Doxygen ---------------------------------
add_subdirectory(../doc ../doc)

#---------------------------------------------------------------
#--------------------- build main project ----------------------
#---------------------------------------------------------------

#--------------------- set output directory  -------------------
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${JAVA_MAIN_RESOURCE_PATH})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${JAVA_MAIN_RESOURCE_PATH})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${JAVA_MAIN_RESOURCE_PATH})

#--------------------- add source files to project -------------
#add the headers into the project
include_directories(${CPP_MAIN_SRC_PATH}/include)
include_directories(${CPP_MAIN_SRC_PATH}/include/spatialOrganization)
include_directories(${CPP_MAIN_SRC_PATH}/swig)

#--------------------- build module cx3d  ----------------
file(GLOB SOURCES "${CPP_MAIN_SRC_PATH}/src/*.cc")
set(CMAKE_SWIG_FLAGS -package ini.cx3d.swig -c++)
set(CMAKE_SWIG_OUTDIR ${JAVA_MAIN_SWIG_PATH}/)

set_source_files_properties(${CPP_MAIN_SRC_PATH}/swig/cx3d.i PROPERTIES CPLUSPLUS ON)
swig_add_module(cx3d java ${CPP_MAIN_SRC_PATH}/swig/cx3d.i ${SOURCES})

#--------------------- build module spatialOrganization  ----------------
file(GLOB SOURCES "${CPP_MAIN_SRC_PATH}/src/spatial_organization/*.cc")
set(CMAKE_SWIG_FLAGS -Wall -package ini.cx3d.swig.spatialOrganization -c++)
set(CMAKE_SWIG_OUTDIR ${JAVA_MAIN_SWIG_PATH}/spatialOrganization)

set_source_files_properties(${CPP_MAIN_SRC_PATH}/swig/spatial_organization.i PROPERTIES CPLUSPLUS ON)
swig_add_module(cx3d_spatialOrganization java ${CPP_MAIN_SRC_PATH}/swig/spatial_organization.i ${SOURCES})
target_link_libraries(cx3d_spatialOrganization ${GMP_LIBRARY})

#---------------------------------------------------------------
#--------------------- build test project ----------------------
#---------------------------------------------------------------
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${JAVA_TEST_RESOURCE_PATH})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${JAVA_TEST_RESOURCE_PATH})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${JAVA_TEST_RESOURCE_PATH})

#--------------------- add source files to project -------------
#add the headers into the project
include_directories(${CPP_TEST_SRC_PATH}/include)
include_directories(${CPP_TEST_SRC_PATH}/include/spatialOrganization)
include_directories(${CPP_TEST_SRC_PATH}/swig)

#--------------------- build module cx3d  ----------------
file(GLOB SOURCES "${CPP_TEST_SRC_PATH}/src/*.cc")
set(CMAKE_SWIG_FLAGS -package ini.cx3d.swig -c++)
set(CMAKE_SWIG_OUTDIR ${JAVA_TEST_SWIG_PATH}/)

set_source_files_properties(${CPP_TEST_SRC_PATH}/swig/cx3d_test.i PROPERTIES CPLUSPLUS ON)
swig_add_module(cx3d_test java ${CPP_TEST_SRC_PATH}/swig/cx3d_test.i ${SOURCES})
target_link_libraries(cx3d_test ${GMP_LIBRARY})
