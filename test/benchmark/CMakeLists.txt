cmake_minimum_required(VERSION 3.2.0)

project(biodynamo-benchmark)

find_package(BioDynaMo REQUIRED)
include(${BDM_USE_FILE})
include_directories("src")
ExternalProject_Add(
	gbench 
  	URL "${CMAKE_SOURCE_DIR}../../third_party/benchmark.zip"
  	PREFIX "${CMAKE_CURRENT_BINARY_DIR}/benchmark"
		CMAKE_CACHE_ARGS
    	-DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
    	-DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
  	INSTALL_COMMAND ""
	BUILD_BYPRODUCTS "${CMAKE_BINARY_DIR}/benchmark/src/gbench-build/src/libbenchmark.a"}
)
ExternalProject_Get_Property(gbench source_dir binary_dir)
add_library(libbenchmark IMPORTED STATIC GLOBAL)
add_dependencies(libbenchmark gbench)
set_target_properties(libbenchmark PROPERTIES
    IMPORTED_LOCATION "${binary_dir}/src/libbenchmark.a"
    IMPORTED_LINK_INTERFACE_LIBRARIES "${CMAKE_THREAD_LIBS_INIT}"
)
include_directories("${CMAKE_BINARY_DIR}/benchmark/src/gbench/src/")

file(GLOB SOURCES ${CMAKE_SOURCE_DIR}/bench-main.cc)
file(GLOB HEADERS ${CMAKE_SOURCE_DIR}/../../demo/tumor_concept/src/*.h)
bdm_add_executable(biodynamo-benchmark
                        SOURCES ${SOURCES}
                        HEADERS ${HEADERS}
                        LIBRARIES ${BDM_REQUIRED_LIBRARIES} benchmark)

add_custom_target(run-benchmark COMMAND ${CMAKE_BINARY_DIR}/bin/biodynamo-benchmark)
add_dependencies(run-benchmark biodynamo-benchmark)