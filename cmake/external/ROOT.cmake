include(ExternalProject)

# Directory in which root will be donwloaded first (the path
# should be something like <build_dir>/third_party/...).
SET(ROOT_SOURCE_DIR "${CMAKE_THIRD_PARTY_DIR}")

file(DOWNLOAD http://cern.ch/biodynamo-lfs/third-party/${DETECTED_OS}/root.tar.gz
        ${ROOT_SOURCE_DIR}/root.tar.gz
        EXPECTED_HASH SHA256=${${DETECTED_OS}-ROOT}
        SHOW_PROGRESS
        INACTIVITY_TIMEOUT 20
        STATUS ROOT_DOWNLOAD_STATUS)

# Check if the download worked properly.
LIST(GET ROOT_DOWNLOAD_STATUS 0 DOWNLOAD_STATUS)
IF (NOT ${DOWNLOAD_STATUS} EQUAL 0)
  MESSAGE( FATAL_ERROR "\nWe were unable to download ROOT. This may be caused by several reason, like \
network error connections or just temporary network failure. Please retry again in a \
few minutes by deleting all the contents of the build directory and by issuing again \
the 'cmake' command.\n")
ENDIF()

file(MAKE_DIRECTORY ${ROOT_SOURCE_DIR}/root)
execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf ${ROOT_SOURCE_DIR}/root.tar.gz
        WORKING_DIRECTORY ${ROOT_SOURCE_DIR}/root)

# Run again find_package in order to find ROOT
find_package(ROOT COMPONENTS Geom Gui)

# Set ROOTSYS variable
string(REGEX REPLACE "/include$" "" TMP_ROOT_PATH ${ROOT_INCLUDE_DIRS})
set(ENV{ROOTSYS} ${TMP_ROOT_PATH})

# Set LD_LIBRARY_PATH variable
set(ENV{LD_LIBRARY_PATH} "$ENV{ROOTSYS}/lib:$ENV{LD_LIBRARY_PATH}")

# Since the user did not set the $ROOTSYS variable, then we need to source
# the ROOT environment every time we need it.
source_root_file(${PROJECT_BINARY_DIR})
set(CUSTOM_ROOT_SOURCE_ENV TRUE PARENT_SCOPE)
set(CUSTOM_ROOT_SOURCE_ENV_COMMAND . ${PROJECT_BINARY_DIR}/source_root_auto.sh PARENT_SCOPE)
