include(utils)

# Directory in which ROOT will be downloaded
SET(ROOT_SOURCE_DIR "${CMAKE_THIRD_PARTY_DIR}/root")

set(ROOT_TAR_FILE root_v6.22.06_python3.9_${DETECTED_OS_VERS}.tar.gz)
if(APPLE)
  # Redirect macOS versions 11.X to ROOT nightlies because after the recent 
  # XCode 13.1 update our pre-built versions became effectively useless.
  if("${DETECTED_OS_VERS}" STREQUAL "osx-11.1-arm64" OR
     "${DETECTED_OS_VERS}" STREQUAL "osx-11.2-arm64" OR
     "${DETECTED_OS_VERS}" STREQUAL "osx-11.3-arm64" OR
     "${DETECTED_OS_VERS}" STREQUAL "osx-11.4-arm64" OR
     "${DETECTED_OS_VERS}" STREQUAL "osx-11.5-arm64" OR
     "${DETECTED_OS_VERS}" STREQUAL "osx-11.5-arm64" OR
     "${DETECTED_OS_VERS}" STREQUAL "osx-11.6-arm64")
    string(SUBSTRING "${DETECTED_OS_VERS}" 4 4 MACOS_VERSION_NUMBER)
    message(STATUS "Identified macOS ${MACOS_VERSION_NUMBER} arm64")
    set(ROOT_TAR_FILE "https://root.cern/download/nightly/root_v6.25.01.macos-${MACOS_VERSION_NUMBER}-arm64-clang120.tar.gz")
    set(VERIFY_DOWNLOAD FALSE)
    set(ROOT_NIGHTLY TRUE)
  endif()
  if("${DETECTED_OS_VERS}" STREQUAL "osx-11.0-i386" OR
     "${DETECTED_OS_VERS}" STREQUAL "osx-11.2-i386" OR
     "${DETECTED_OS_VERS}" STREQUAL "osx-11.3-i386" OR
     "${DETECTED_OS_VERS}" STREQUAL "osx-11.4-i386" OR
     "${DETECTED_OS_VERS}" STREQUAL "osx-11.5-i386" OR
     "${DETECTED_OS_VERS}" STREQUAL "osx-11.6-i386")
    string(SUBSTRING "${DETECTED_OS_VERS}" 4 4 MACOS_VERSION_NUMBER)
    message(STATUS "Identified macOS ${MACOS_VERSION_NUMBER} x86_64")
    set(ROOT_TAR_FILE "https://root.cern/download/nightly/root_v6.25.01.macos-${MACOS_VERSION_NUMBER}-x86_64-clang120.tar.gz")
    set(VERIFY_DOWNLOAD FALSE)
    set(ROOT_NIGHTLY TRUE)
  endif()
  if("${DETECTED_OS_VERS}" STREQUAL "osx-11.6.1-arm64" OR 
     "${DETECTED_OS_VERS}" STREQUAL "osx-12.1-arm64" OR
     "${DETECTED_OS_VERS}" STREQUAL "osx-11.1-i386" OR
     "${DETECTED_OS_VERS}" STREQUAL "osx-12.1-i386" OR
     "${DETECTED_OS_VERS}" STREQUAL "osx-11.6.1-i386")
    message(FATAL_ERROR "Your macOS version is currently not supported. Please contact the developers.")
  endif()
  if("${DETECTED_OS_VERS}" MATCHES "^osx-12")
    set(ROOT_TAR_FILE root_v6.25.01_cxx14_python3.9_${DETECTED_OS_VERS}.tar.gz)
  endif()
endif()

download_verify_extract(
  ${ROOT_TAR_FILE}
  ${ROOT_SOURCE_DIR}
  ${${DETECTED_OS_VERS}-ROOT}
)



# Run again find_package in order to find ROOT
message(STATUS "Searching Root.")
find_package(ROOT COMPONENTS Geom Gui GenVector REQUIRED)
message(STATUS "Found Root.")

# Set ROOTSYS variable
string(REGEX REPLACE "/include$" "" TMP_ROOT_PATH ${ROOT_INCLUDE_DIRS})
set(ENV{ROOTSYS} ${TMP_ROOT_PATH})

# Set ROOT_CONFIG_EXECUTABLE variable
find_program(ROOT_CONFIG_EXECUTABLE NAMES root-config HINTS "${TMP_ROOT_PATH}/bin")
SET(ROOT_CONFIG_EXECUTABLE ${ROOT_CONFIG_EXECUTABLE} PARENT_SCOPE)
