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
ExternalProject_Add(
	gbench 
  	URL "${CMAKE_SOURCE_DIR}/third_party/benchmark.zip"
  	PREFIX "${CMAKE_CURRENT_BINARY_DIR}/benchmark"
    CMAKE_ARGS
      -DBENCHMARK_ENABLE_TESTING:BOOL=OFF
      -DBENCHMARK_ENABLE_INSTALL:BOOL=OFF
      -DBENCHMARK_ENABLE_GTEST_TESTS:BOOL=OFF
		CMAKE_CACHE_ARGS
    	-DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
    	-DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
      -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=true
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
include_directories("${CMAKE_SOURCE_DIR}/test/benchmark/demo")


set(LAUNCHER ${CMAKE_BINARY_DIR}/launcher.sh)
add_custom_target(run-benchmarks
                  COMMAND ${LAUNCHER$} ${CMAKE_BINARY_DIR}/bin/biodynamo-benchmark --benchmark_format=json --benchmark_out=../test/benchmark/soma_clusterin0.json --benchmark_filter=SomaClustering0
                  COMMAND ${LAUNCHER$} ${CMAKE_BINARY_DIR}/bin/biodynamo-benchmark --benchmark_format=json --benchmark_out=../test/benchmark/soma_clustering1.json --benchmark_filter=SomaClustering1
                  COMMAND ${LAUNCHER$} ${CMAKE_BINARY_DIR}/bin/biodynamo-benchmark --benchmark_format=json --benchmark_out=../test/benchmark/tumor_concept0.json --benchmark_filter=TumorConcept0
                  COMMAND ${LAUNCHER$} ${CMAKE_BINARY_DIR}/bin/biodynamo-benchmark --benchmark_format=json --benchmark_out=../test/benchmark/tumor_concept1.json --benchmark_filter=TumorConcept1)


file(GLOB_RECURSE BENCH_HEADERS ${CMAKE_SOURCE_DIR}/test/benchmark/*.h)
file(GLOB_RECURSE BENCH_SOURCES ${CMAKE_SOURCE_DIR}/test/benchmark/*.cc)
file(GLOB_RECURSE DEMO_HEADERS ${CMAKE_SOURCE_DIR}/demo/soma_clustering/*.h 
                               ${CMAKE_SOURCE_DIR}/demo/tumor_concept/*.h)
include_directories("${CMAKE_SOURCE_DIR}/demo/tumor_concept/src")
include_directories("${CMAKE_SOURCE_DIR}/demo/soma_clustering/src")
include_directories("${CMAKE_SOURCE_DIR}/build/benchmark/src/gbench/include/benchmark")
bdm_add_executable(biodynamo-benchmark
                   HEADERS ${DEMO_HEADERS} ${BENCH_HEADERS}
                   SOURCES ${BENCH_SOURCES}
                   LIBRARIES ${BDM_REQUIRED_LIBRARIES} ${FS_LIB} biodynamo libbenchmark)
add_dependencies(run-benchmarks biodynamo-benchmark)

