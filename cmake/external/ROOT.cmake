include(utils)

# Directory in which ROOT will be downloaded
SET(ROOT_SOURCE_DIR "${CMAKE_THIRD_PARTY_DIR}/root")

# Which tarball belongs to this platform, and the key to verify it against.
# The mapping lives in cmake/utils.cmake so that verify_ROOT() can reuse it.
bdm_root_platform(ROOT_TAR_FILE ROOT_SHA_KEY)
set(ROOT_SHA ${${ROOT_SHA_KEY}})

if("${ROOT_SHA}" STREQUAL "")
  message(FATAL_ERROR "BioDynaMo does not ship a ROOT build for this platform "
    "(no SHA256 digest registered under the key '${ROOT_SHA_KEY}'). "
    "Please install ROOT yourself and source it before running cmake.")
endif()

message(STATUS "Using  ROOT tarball    : ${ROOT_TAR_FILE}")
message(STATUS "Using  ROOT source dir : ${ROOT_SOURCE_DIR}")
message(STATUS "Using  ROOT SHA key    : ${ROOT_SHA_KEY}")
message(STATUS "Verify ROOT SHA        : ${ROOT_SHA}")

download_verify_extract(
  http://cern.ch/biodynamo-lfs/third-party/${ROOT_TAR_FILE}
  ${ROOT_SOURCE_DIR}
  ${ROOT_SHA}
)

# NOTE: the install name fixup for these tarballs (fix_root_install_names) is
# invoked from the top level CMakeLists.txt after verify_ROOT(), so that it also
# repairs a tree that was downloaded by an earlier run of cmake.

# Run again find_package in order to find ROOT
find_package(ROOT COMPONENTS Geom Gui GenVector REQUIRED)

# Set ROOTSYS variable
string(REGEX REPLACE "/include$" "" TMP_ROOT_PATH "${ROOT_INCLUDE_DIRS}")
set(ENV{ROOTSYS} ${TMP_ROOT_PATH})

# Set ROOT_CONFIG_EXECUTABLE variable
find_program(ROOT_CONFIG_EXECUTABLE NAMES root-config HINTS "${TMP_ROOT_PATH}/bin")
SET(ROOT_CONFIG_EXECUTABLE ${ROOT_CONFIG_EXECUTABLE} PARENT_SCOPE)
