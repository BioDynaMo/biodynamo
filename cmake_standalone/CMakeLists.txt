cmake_minimum_required(VERSION 2.8.9)
project(directory_test)
#set(CMAKE_BUILD_TYPE Release)

# include custom cmake files
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "../cmake/modules/")

#--------------------- path definitions ----------------------
#main
SET(CPP_MAIN_SRC_PATH ../src/main/cpp)
SET(JAVA_MAIN_RESOURCE_PATH ../src/main/resources)
SET(JAVA_MAIN_SWIG_PATH ../src/main/java/ini/cx3d/swig)
#test
SET(CPP_TEST_SRC_PATH ../src/test/cpp)
SET(JAVA_TEST_RESOURCE_PATH ../src/test/resources)
SET(JAVA_TEST_SWIG_PATH ../src/test/java/ini/cx3d/swig)

#--------------------- set compiler flags -------------
SET(GCC_COMPILE_FLAGS "-Wall -Wextra -g")
SET(GCC_LINK_FLAGS    "")
# append
SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COMPILE_FLAGS}" )
SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_LINK_FLAGS}")

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

#--------------------- setup GMP ------------------------------
FIND_PACKAGE(GMP REQUIRED)
INCLUDE_DIRECTORIES(${GMP_INCLUDE_DIR})


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
include_directories(${CPP_MAIN_SRC_PATH}/include/physics)
include_directories(${CPP_MAIN_SRC_PATH}/swig)

#add the sources into the project
file(GLOB SOURCES "${CPP_MAIN_SRC_PATH}/src/*.cc")
file(GLOB SOURCES "${CPP_MAIN_SRC_PATH}/src/spatial_organization/*.cc")
file(GLOB SOURCES "${CPP_MAIN_SRC_PATH}/src/physics/*.cc")
file(GLOB SOURCES "${CPP_MAIN_SRC_PATH}/src/local_biology/*.cc")
file(GLOB SOURCES "${CPP_MAIN_SRC_PATH}/src/synapse/*.cc")
file(GLOB SOURCES "${CPP_MAIN_SRC_PATH}/src/cells/*.cc")
file(GLOB SOURCES "${CPP_MAIN_SRC_PATH}/src/simulation/*.cc")

#--------------------- build cx3d-cpp  ----------------
#add_library(cx3d-cpp SHARED ${SOURCES})
add_executable(cx3d-cpp ${SOURCES})
target_link_libraries(cx3d-cpp ${GMP_LIBRARY})
