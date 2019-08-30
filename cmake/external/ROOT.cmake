include(utils)

# Directory in which root will be downloaded first (the path
# should be something like <build_dir>/third_party/...).
SET(ROOT_SOURCE_DIR "${CMAKE_THIRD_PARTY_DIR}")

# Download the package and retry if something went wrong
download_retry(
  http://cern.ch/biodynamo-lfs/third-party/${DETECTED_OS}/root.tar.gz
  ${ROOT_SOURCE_DIR}/root.tar.gz
  ${${DETECTED_OS}-ROOT}
  "ROOT"
)

file(MAKE_DIRECTORY ${ROOT_SOURCE_DIR}/root)
execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf ${ROOT_SOURCE_DIR}/root.tar.gz
        WORKING_DIRECTORY ${ROOT_SOURCE_DIR}/root)

# Run again find_package in order to find ROOT
find_package(ROOT COMPONENTS Geom Gui PATHS ${ROOT_SOURCE_DIR}/root/cmake)

# Set ROOTSYS variable
string(REGEX REPLACE "/include$" "" TMP_ROOT_PATH ${ROOT_INCLUDE_DIRS})
set(ENV{ROOTSYS} ${TMP_ROOT_PATH})
