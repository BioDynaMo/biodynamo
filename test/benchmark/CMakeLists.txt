cmake_minimum_required(VERSION 3.2.0)

project(biodynamo-benchmark)

find_package(BioDynaMo REQUIRED)
find_package(benchmark REQUIRED)
include("${BDM_USE_FILE}")
include_directories("src")

target_link_libraries(benchmark::benchmark)

file(GLOB_RECURSE HEADERS
						../../demo/tumor_concept/src/*.h
#						../../demo/soma_clustering/src/*.h
						)
#set(HEADERS ../../demo/soma_clustering/src/soma_clustering.h
#			../../demo/soma_clustering/src/my_cell.h)
file(GLOB_RECURSE SOURCES main.cc)

bdm_add_executable(biodynamo-benchmark
    	            HEADERS "${HEADERS}"
        	       SOURCES "${SOURCES}"
                   LIBRARIES ${BDM_REQUIRED_LIBRARIES} benchmark)
add_custom_target(run-benchmark COMMAND ${CMAKE_BINARY_DIR}/biodynamo-benchmark)
add_dependencies(run-benchmark biodynamo-benchmark)