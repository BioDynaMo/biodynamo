include(utils)

# Directory in which root will be downloaded first (the path
# should be something like <build_dir>/third_party/...).
SET(ROOT_SOURCE_DIR "${CMAKE_THIRD_PARTY_DIR}")

if(${DETECTED_OS} MATCHES "osx")
  set(ROOT_TAR_FILE root_v6-18-04_${DETECTED_OS}.tar.gz)
else()
  set(ROOT_TAR_FILE root_v6-18-04_python3_${DETECTED_OS}.tar.gz)
endif()
download_verify_extract(
  http://cern.ch/biodynamo-lfs/third-party/${ROOT_TAR_FILE}
  ${ROOT_SOURCE_DIR}/root
  ${${DETECTED_OS}-ROOT}
)

# Run again find_package in order to find ROOT
find_package(ROOT COMPONENTS Geom Gui GenVector REQUIRED)

# Set ROOTSYS variable
string(REGEX REPLACE "/include$" "" TMP_ROOT_PATH ${ROOT_INCLUDE_DIRS})
set(ENV{ROOTSYS} ${TMP_ROOT_PATH})

# Set ROOT_CONFIG_EXECUTABLE variable
find_program(ROOT_CONFIG_EXECUTABLE NAMES root-config HINTS "${TMP_ROOT_PATH}/bin")
SET(ROOT_CONFIG_EXECUTABLE ${ROOT_CONFIG_EXECUTABLE} PARENT_SCOPE)
