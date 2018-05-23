if(NOT GIT_FOUND)
  message(FATAL_ERROR "Git not found.")
endif()

# set and create directory for building snap package already here. Required to
# write the snapcraft.yaml
set(SNAP_DIR ${CMAKE_BINARY_DIR}/build-snap)
file(MAKE_DIRECTORY ${SNAP_DIR})

execute_process(
     COMMAND "cmake/version/generate_version_files.py" ${GIT_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}
     WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

add_custom_target(update-version-info
     COMMAND "cmake/version/generate_version_files.py" ${GIT_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}
     WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
