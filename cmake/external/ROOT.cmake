include(utils)

# Directory in which root will be downloaded first (the path
# should be something like <build_dir>/third_party/...).
SET(ROOT_SOURCE_DIR "${CMAKE_THIRD_PARTY_DIR}")

# FIXME remove this if statement (else branch) after ROOT has been updated for MacOS too
if (LINUX)
  set(ROOT_TAR_FILE root_v6-18-04_${DETECTED_OS}.tar.gz)
  download_verify_extract(
    http://cern.ch/biodynamo-lfs/third-party/${ROOT_TAR_FILE}
    ${ROOT_SOURCE_DIR}/root
    ${${DETECTED_OS}-ROOT}
  )
else()
  # Still using the old structure
  download_verify_extract(
    http://cern.ch/biodynamo-lfs/third-party/${DETECTED_OS}/root.tar.gz
    ${ROOT_SOURCE_DIR}/root
    ${${DETECTED_OS}-ROOT}
  )
endif()

# Run again find_package in order to find ROOT
find_package(ROOT COMPONENTS Geom Gui PATHS ${ROOT_SOURCE_DIR}/root/cmake)

# Set ROOTSYS variable
string(REGEX REPLACE "/include$" "" TMP_ROOT_PATH ${ROOT_INCLUDE_DIRS})
set(ENV{ROOTSYS} ${TMP_ROOT_PATH})
