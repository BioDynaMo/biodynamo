cmake_minimum_required(VERSION 2.8.9)
project(directory_test)
#set(CMAKE_BUILD_TYPE Release)

#--------------------- path definitions ----------------------
#main
SET(CPP_MAIN_SRC_PATH ../src/main/cpp)
SET(JAVA_MAIN_RESOURCE_PATH ../src/main/resources)
SET(JAVA_MAIN_SWIG_PATH ../src/main/java/ini/cx3d/swig)
#test
SET(CPP_TEST_SRC_PATH ../src/test/cpp)
SET(JAVA_TEST_RESOURCE_PATH ../src/test/resources)
SET(JAVA_TEST_SWIG_PATH ../src/test/java/ini/cx3d/swig)

#--------------------- check if compiler supports C++11 -------------
#http://stackoverflow.com/questions/10851247/how-to-activate-c-11-in-cmake
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

#--------------------- setup swig ------------------------------
#swig Java wrappers
FIND_PACKAGE(SWIG REQUIRED)
INCLUDE(${SWIG_USE_FILE})

FIND_PACKAGE(JNI REQUIRED)
INCLUDE_DIRECTORIES(${JAVA_INCLUDE_PATH})       #path to jni.h
INCLUDE_DIRECTORIES(${JAVA_INCLUDE_PATH2})      #path to jni_md.h
INCLUDE_DIRECTORIES(${SWIG_DIR}/java)           #path to swig java includes


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

#add the sources into the project
file(GLOB SOURCES "${CPP_MAIN_SRC_PATH}/src/*.cc")

#--------------------- build module cx3d  ----------------
SET(CMAKE_SWIG_FLAGS -package ini.cx3d.swig -c++)
SET(CMAKE_SWIG_OUTDIR ${JAVA_MAIN_SWIG_PATH}/)

SET_SOURCE_FILES_PROPERTIES(${CPP_MAIN_SRC_PATH}/swig/cx3d.i PROPERTIES CPLUSPLUS ON)
SWIG_ADD_MODULE(cx3d java ${CPP_MAIN_SRC_PATH}/swig/cx3d.i ${SOURCES})


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

#add the sources into the project
file(GLOB SOURCES "${CPP_TEST_SRC_PATH}/src/*.cc")

#--------------------- build module cx3d  ----------------
SET(CMAKE_SWIG_FLAGS -package ini.cx3d.swig -c++)
SET(CMAKE_SWIG_OUTDIR ${JAVA_TEST_SWIG_PATH}/)

SET_SOURCE_FILES_PROPERTIES(${CPP_TEST_SRC_PATH}/swig/cx3d_test.i PROPERTIES CPLUSPLUS ON)
SWIG_ADD_MODULE(cx3d_test java ${CPP_TEST_SRC_PATH}/swig/cx3d_test.i ${SOURCES})
