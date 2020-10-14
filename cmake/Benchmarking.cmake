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

function(bdm_add_demo_benchmark DEMO_NAME)
  file(GLOB BENCH_HEADERS ${CMAKE_SOURCE_DIR}/test/benchmark/demo/${DEMO_NAME}_bm.h)
  file(GLOB BENCH_SOURCES ${CMAKE_SOURCE_DIR}/test/benchmark/demo/${DEMO_NAME}_bm.cc)
  file(GLOB DEMO_HEADERS ${CMAKE_SOURCE_DIR}/demo/${DEMO_NAME}/src/*.h)
  include_directories("${CMAKE_SOURCE_DIR}/demo/${DEMO_NAME}/src")
  include_directories("${CMAKE_SOURCE_DIR}/build/benchmark/src/gbench/include/benchmark")
  bdm_add_executable(biodynamo-benchmark-${DEMO_NAME}
                     HEADERS ${DEMO_HEADERS} ${BENCH_HEADERS}
                     SOURCES ${BENCH_SOURCES}
                     LIBRARIES ${BDM_REQUIRED_LIBRARIES} ${FS_LIB} biodynamo libbenchmark)
  add_dependencies(run-benchmark biodynamo-benchmark-${DEMO_NAME})
endfunction(bdm_add_demo_benchmark)

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
add_custom_target(run-benchmark 
                  COMMAND ${LAUNCHER$} {CMAKE_BINARY_DIR}/bin/biodynamo-benchmark-tumor_concept
                  COMMAND ${LAUNCHER$} {CMAKE_BINARY_DIR}/bin/biodynamo-benchmark-soma_clustering)

bdm_add_demo_benchmark(tumor_concept)
bdm_add_demo_benchmark(soma_clustering)


