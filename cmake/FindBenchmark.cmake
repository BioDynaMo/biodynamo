# Module for locating libbenchmark
#
# Read-only variables:
#   BENCHMARK_FOUND
#     Indicates that the library has been found.
#
#   BENCHMARK_INCLUDE_DIR
#     Points to the libbenchmark include directory.
#
#   BENCHMARK_LIBRARY_DIR
#     Points to the directory that contains the libraries.
#     The content of this variable can be passed to link_directories.
#
#   BENCHMARK_LIBRARY
#     Points to the libbenchmark that can be passed to target_link_libararies.
#
# Copyright (c) 2020 Robert Harakaly

find_path(BENCHMARK_ROOT_DIR
  NAMES include/benchmark/benchmark.h
  PATHS ENV BENCHMARK_ROOT
  DOC "BENCHMARK root directory")

find_path(BENCHMARK_INCLUDE_DIR
  NAMES benchmark.h
  HINTS ${NUMA_ROOT_DIR}
  PATH_SUFFIXES include
  DOC "BENCHMARK include directory")

find_library(BENCHMARK_LIBRARY
  NAMES benchmark
  HINTS ${BENCHMARK_ROOT_DIR}
  DOC "BENCHMARK library")

if (BENCHMARK_LIBRARY)
    get_filename_component(BENCHMARK_LIBRARY_DIR ${BENCHMARK_LIBRARY} DIRECTORY)
endif()

mark_as_advanced(BENCHMARK_INCLUDE_DIR BENCHMARK_LIBRARY_DIR BENCHMARK_LIBRARY)

INCLUDE(FindPackageHandleStandardArgs)
find_package_handle_standard_args(benchmark REQUIRED_VARS BENCHMARK_LIBRARY BENCHMARK_INCLUDE_DIR BENCHMARK_LIBRARY_DIR)
