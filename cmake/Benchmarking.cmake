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
  benchmark
  URL "${CMAKE_SOURCE_DIR}/third_party/benchmark.zip"
  PREFIX "${CMAKE_CURRENT_BINARY_DIR}/benchmark"
  CMAKE_CACHE_ARGS
    -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
    -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
  INSTALL_COMMAND "" # Disable install step
  BUILD_BYPRODUCTS "${CMAKE_BINARY_DIR}/benchmark/build/src/libbenchmark.a"
)
ExternalProject_Get_Property(benchmark source_dir binary_dir)

# Create a libgtest target to be used as a dependency by benchmark program
add_library(libbenchmark IMPORTED STATIC GLOBAL)
add_dependencies(libbenchmark benchmark)
set_target_properties(libbenchmark PROPERTIES
    IMPORTED_LOCATION "${binary_dir}/libbenchmark.a"
    IMPORTED_LINK_INTERFACE_LIBRARIES "${CMAKE_THREAD_LIBS_INIT}"
)

# add include directories for benchmark
include_directories("${CMAKE_BINARY_DIR}/benchmark/include/benchmark/")

# create target that shows the test ouput on failure
add_custom_target(run-benchmark COMMAND ${CMAKE_BINARY_DIR}/bin/biodynamo-benchmark)

