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
  URL "${CMAKE_SOURCE_DIR}/third_party/benchmark-1.5.5.zip"
  PREFIX "${CMAKE_CURRENT_BINARY_DIR}/gbench"
  CMAKE_ARGS
    -DBENCHMARK_ENABLE_TESTING:BOOL=OFF
    -DBENCHMARK_ENABLE_INSTALL:BOOL=OFF
  CMAKE_CACHE_ARGS
    -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
    -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
    -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=true
  INSTALL_COMMAND cp -r "${CMAKE_CURRENT_BINARY_DIR}/gbench/src/gbench/include/benchmark" "${CMAKE_CURRENT_BINARY_DIR}/include"
  # Ugly but necessary, in future versions one can use ${binary_dir}
  # in BUILD_BYPRODUCTS
  #BUILD_BYPRODUCTS "${binary_dir}/src/libbenchmark.a"
  BUILD_BYPRODUCTS "${CMAKE_BINARY_DIR}/gbench/src/gbench-build/src/libbenchmark.a"
)
ExternalProject_Get_Property(gbench source_dir binary_dir)

# Create a libbenchmark target to be used as a dependency by benchmark program
add_library(libbenchmark IMPORTED STATIC GLOBAL)
add_dependencies(libbenchmark gbench)
set_target_properties(libbenchmark PROPERTIES
  IMPORTED_LOCATION "${binary_dir}/src/libbenchmark.a"
  IMPORTED_LINK_INTERFACE_LIBRARIES "${CMAKE_THREAD_LIBS_INIT}"
)

# add include directories for benchmark
include_directories("${CMAKE_BINARY_DIR}/gbench/src/gbench/include")

# create target that runs the benchmarks
set(LAUNCHER ${CMAKE_BINARY_DIR}/launcher.sh)
add_custom_target(run-benchmarks
                  COMMAND ${LAUNCHER} ${CMAKE_BINARY_DIR}/bin/biodynamo-benchmark --benchmark_repetitions=1 --benchmark_format=json --benchmark_out=gbench/results.json
                  COMMAND ${LAUNCHER} ${CMAKE_BINARY_DIR}/bench_version.sh
                  COMMAND ${LAUNCHER} ${CMAKE_BINARY_DIR}/bench_gen_html_page.py
)

# create biodyname-benchmark executable
file(GLOB_RECURSE BENCH_HEADERS ${CMAKE_SOURCE_DIR}/benchmark/*.h)
file(GLOB_RECURSE BENCH_SOURCES ${CMAKE_SOURCE_DIR}/benchmark/*.cc)
file(GLOB_RECURSE DEMO_HEADERS ${CMAKE_SOURCE_DIR}/demo/soma_clustering/src/*.h 
                               ${CMAKE_SOURCE_DIR}/demo/tumor_concept/src/*.h)
include_directories("${CMAKE_SOURCE_DIR}/benchmark")
include_directories("${CMAKE_SOURCE_DIR}/demo/tumor_concept/src")
include_directories("${CMAKE_SOURCE_DIR}/demo/soma_clustering/src")
include_directories("${CMAKE_BINARY_DIR}/gbench/src/gbench/src")
bdm_add_executable(biodynamo-benchmark
                   HEADERS ${DEMO_HEADERS} ${BENCH_HEADERS}
                   SOURCES ${BENCH_SOURCES}
                   LIBRARIES ${BDM_REQUIRED_LIBRARIES} ${FS_LIB} biodynamo libbenchmark
)
add_dependencies(run-benchmarks biodynamo-benchmark)
