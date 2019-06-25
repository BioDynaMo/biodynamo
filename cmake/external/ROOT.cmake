include(ExternalProject)

# Directory in which root will be donwloaded first (the path
# should be something like <build_dir>/third_party/...).
SET(ROOT_SOURCE_DIR "${CMAKE_BINARY_DIR}/third_party")

file(DOWNLOAD http://cern.ch/biodynamo-lfs/third-party/${DETECTED_OS}/root.tar.gz ${ROOT_SOURCE_DIR}/root.tar.gz
        SHOW_PROGRESS)
file(MAKE_DIRECTORY ${ROOT_SOURCE_DIR}/root)
execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf ${ROOT_SOURCE_DIR}/root.tar.gz
        WORKING_DIRECTORY ${ROOT_SOURCE_DIR}/root)
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${ROOT_SOURCE_DIR}/root ${THIRD_PARTY_DIR}/root)

# Run again find_package in order to find ROOT
find_package(ROOT COMPONENTS Geom Gui)

# Set ROOTSYS variable
SET(ENV{ROOTSYS} "${THIRD_PARTY_DIR}/root" PARENT_SCOPE)

# Set LD_LIBRARY_PATH variable
set(ENV{LD_LIBRARY_PATH} "$ENV{ROOTSYS}/lib:$ENV{LD_LIBRARY_PATH}" PARENT_SCOPE)

# Since the user did not set the $ROOTSYS variable, then we need to source
# the ROOT environment every time we need it.
source_root_file(${PROJECT_BINARY_DIR})
set(CUSTOM_ROOT_SOURCE_ENV TRUE PARENT_SCOPE)
set(CUSTOM_ROOT_SOURCE_ENV_COMMAND . ${PROJECT_BINARY_DIR}/source_root_auto.sh PARENT_SCOPE)

