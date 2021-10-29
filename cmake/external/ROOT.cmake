include(utils)

# Directory in which ROOT will be downloaded
SET(ROOT_SOURCE_DIR "${CMAKE_THIRD_PARTY_DIR}/root")

set(ROOT_TAR_FILE root_v6.22.06_python3.9_${DETECTED_OS_VERS}.tar.gz)
if(APPLE)
  # On Apple macOS 11.4 and 11.5 arm64 need ROOT v6.24 which has TBB for M1
  # (remove later when upgrading other platforms to ROOT v6.24)
  if("${DETECTED_OS_VERS}" STREQUAL "osx-11.4-arm64" OR
     "${DETECTED_OS_VERS}" STREQUAL "osx-11.5-arm64" OR
     "${DETECTED_OS_VERS}" STREQUAL "osx-11.6-arm64")
    set(ROOT_TAR_FILE root_v6.24.00_python3.9_${DETECTED_OS_VERS}.tar.gz)
  endif()
  if("${DETECTED_OS_VERS}" MATCHES "^osx-12")
    set(ROOT_TAR_FILE root_v6.25.01_cxx14_python3.9_${DETECTED_OS_VERS}.tar.gz)
  endif()
endif()

download_verify_extract(
  http://cern.ch/biodynamo-lfs/third-party/${ROOT_TAR_FILE}
  ${ROOT_SOURCE_DIR}
  ${${DETECTED_OS_VERS}-ROOT}
)

# Run again find_package in order to find ROOT
find_package(ROOT COMPONENTS Geom Gui GenVector REQUIRED)

# Set ROOTSYS variable
string(REGEX REPLACE "/include$" "" TMP_ROOT_PATH ${ROOT_INCLUDE_DIRS})
set(ENV{ROOTSYS} ${TMP_ROOT_PATH})

# Set ROOT_CONFIG_EXECUTABLE variable
find_program(ROOT_CONFIG_EXECUTABLE NAMES root-config HINTS "${TMP_ROOT_PATH}/bin")
SET(ROOT_CONFIG_EXECUTABLE ${ROOT_CONFIG_EXECUTABLE} PARENT_SCOPE)
