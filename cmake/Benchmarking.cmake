# -----------------------------------------------------------------------------
#
# Copyright (C) The BioDynaMo Project.
# All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
#
# See the LICENSE file distributed with this work for details.
# See the NOTICE file distributed with this work for additional information
# regarding copyright ownership.
#
# -----------------------------------------------------------------------------

# setup google benchmark
# project(biodynamo-benchmark)
# set(CMAKE_CXX_STANDARD 14)
# find_package(BioDynaMo)
# if (NOT BioDynaMo_FOUND)
# 	message(FATAL_ERROR "BioDynaMo not found.\n")
# endif()
# find_package(benchmark REQUIRED)
# include_directories("src")
# if (NOT benchmark_FOUND)
# 	ExternalProject_Add(
# 		benchmark 
#   	URL "${CMAKE_SOURCE_DIR}/third_party/benchmark.zip"
#   	PREFIX "${CMAKE_CURRENT_BINARY_DIR}/benchmark"
# 		CMAKE_CACHE_ARGS
#     	-DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
#     	-DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
#   	INSTALL_COMMAND ""
# 	)
# 	add_library(libbenchmark IMPORTED STATIC GLOBAL)
# 	add_dependencies(libbenchmark benchmark)
# 	find_package(benchmark REQUIRED)
# endif()

# set(SOURCES test/benchmark/bench-main.cc)
# #set(HEADERS demo/tumor_concept/src/tumor_concept.h
# #			src/biodynamo.h
# #			)
# #file(GLOB_RECURSE HEADERS src/*.h)
# #set(HEADERS build/omp/omp.h)

# add_executable(biodynamo-benchmark 
# 			  ${SOURCES}
# #			  ${HEADERS}
#               )


# #bdm_add_executable(biodynamo-benchmark
# #                   HEADERS ${HEADERS}
# #                   SOURCES ${SOURCES}
# #				   LIBRARIES ${BDM_REQUIRED_LIBRARIES} benchmark)

# target_link_libraries(biodynamo-benchmark benchmark::benchmark)
# #target_link_libraries(biodynamo-benchmark biodynamo)

# add_custom_target(run-benchmark COMMAND ${CMAKE_BINARY_DIR}/bin/biodynamo-benchmark)
# add_dependencies(run-benchmark biodynamo-benchmark)


ExternalProject_Add(
	gbench 
  	URL "${CMAKE_SOURCE_DIR}/third_party/benchmark.zip"
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

set(SOURCES test/benchmark/bench-main.cc)

add_executable(biodynamo-benchmark 
 			  ${SOURCES}
            	)

target_link_libraries(biodynamo-benchmark benchmark)


add_custom_target(run-benchmark COMMAND ${CMAKE_BINARY_DIR}/bin/biodynamo-benchmark)
add_dependencies(run-benchmark biodynamo-benchmark)
						