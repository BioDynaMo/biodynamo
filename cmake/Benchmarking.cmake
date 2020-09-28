find_package(benchmark REQUIRED)
target_link_libraries(benchmark::benchmark)
bdm_add_executable($(TEST_TARGET)
                   HEADERS ${HEADERS}
                   SOURCES ${SOURCES}
                   LIBRARIES ${BDM_REQUIRED_LIBRARIES} benchmark)