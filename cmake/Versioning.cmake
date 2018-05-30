if(NOT GIT_FOUND)
  message(FATAL_ERROR "Git not found.")
endif()

execute_process(
     COMMAND "cmake/version/generate_version_files.py" ${GIT_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}
     WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

add_custom_target(update-version-info
     COMMAND "cmake/version/generate_version_files.py" ${GIT_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}
     WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
