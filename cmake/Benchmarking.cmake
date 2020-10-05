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
project(biodynamo-benchmark)
set(CMAKE_CXX_STANDARD 14)
find_package(benchmark REQUIRED)
if (NOT benchmark_FOUND)
	ExternalProject_Add(
		benchmark 
  	URL "${CMAKE_SOURCE_DIR}/third_party/benchmark.zip"
  	PREFIX "${CMAKE_CURRENT_BINARY_DIR}/benchmark"
		CMAKE_CACHE_ARGS
    	-DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
    	-DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
  	INSTALL_COMMAND ""
	)
	add_library(libbenchmark IMPORTED STATIC GLOBAL)
	add_dependencies(libbenchmark benchmark)
	find_package(benchmark REQUIRED)
endif()

set(SOURCES test/benchmark/main.cc)

add_executable(biodynamo-benchmark 
              ${SOURCES}
              )

target_link_libraries(biodynamo-benchmark benchmark::benchmark)