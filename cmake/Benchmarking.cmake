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

function(bdm_add_bench_executable BENCH_TARGET)
	cmake_parse_arguments(ARG "" "" "SOURCES;HEADERS;LIBRARIES" ${ARGN} )
	bdm_add_executable(${BENCH_TARGET}
                     SOURCES ${ARG_SOURCES}
                     HEADERS ${ARG_HEADERS}
					 LIBRARIES biodynamo libbenchmark ${ARG_LIBRARIES})
endfunction(bdm_add_bench_executable)

add_custom_target(run-benchmark COMMAND ${CMAKE_BINARY_DIR}/bin/biodynamo-benchmark)
add_dependencies(run-benchmark biodynamo-benchmark)
						