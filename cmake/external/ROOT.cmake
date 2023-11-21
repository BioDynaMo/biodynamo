include(utils)

# Directory in which ROOT will be downloaded
SET(ROOT_SOURCE_DIR "${CMAKE_THIRD_PARTY_DIR}/root")

set(ROOT_TAR_FILE root_v6.22.06_python3.9_${DETECTED_OS_VERS}.tar.gz)
set(ROOT_SHA_KEY ${DETECTED_OS_VERS}-ROOT)
if(APPLE)
  if("${DETECTED_OS_VERS}" MATCHES "^osx-14" OR
     "${DETECTED_OS_VERS}" MATCHES "^osx-13" OR
     "${DETECTED_OS_VERS}" MATCHES "^osx-12" OR
     "${DETECTED_OS_VERS}" MATCHES "^osx-11.6" OR
     "${DETECTED_OS_VERS}" MATCHES "^osx-11.7")
    # Test if Xcode is installed via `xcodebuild -version`.
    message(STATUS "##### Checking if XCODE is installed")
    execute_process(COMMAND bash "-c" "xcodebuild -version" ERROR_VARIABLE XCODE_ERROR)
    if("${XCODE_ERROR}" STREQUAL "")
      message(STATUS "##### XCODE is installed")
    else()
      message(FATAL_ERROR "##### XCODE is not installed correctly. Please install XCODE (and then the command line tools). Consult issue #352 and the documentation")
    endif()
    # Determine XCDOE version
    execute_process(COMMAND bash "-c" "xcodebuild -version | sed -En 's/Xcode[[:space:]]+([0-9\.]*)/\\1/p'" OUTPUT_VARIABLE XCODE_VERS)
    message(STATUS "##### XCODE version: ${XCODE_VERS}")
    # Set appropriate ROOT version depending on XCODE version
    if("${XCODE_VERS}" GREATER_EQUAL "14.3")
      message(STATUS "##### Using ROOT builds for XCODE 14.3")
      set(ROOT_TAR_FILE root_v6.29.01_cxx14_python3.9_osx-xcode-14.3-${DETECTED_ARCH}.tar.gz)
      set(ROOT_SHA_KEY osx-xcode-14.3-${DETECTED_ARCH}-ROOT)
    elseif("${XCODE_VERS}" GREATER_EQUAL "14.1")
      message(STATUS "##### Using ROOT builds for XCODE 14.1")
      set(ROOT_TAR_FILE root_v6.26.06_cxx14_python3.9_osx-xcode-14.1-${DETECTED_ARCH}.tar.gz)
      set(ROOT_SHA_KEY osx-xcode-14.1-${DETECTED_ARCH}-ROOT)
    else()
      message(STATUS "##### Using ROOT builds for XCODE 13.1")
      set(ROOT_TAR_FILE root_v6.25.01_cxx14_python3.9_osx-xcode-13.1-${DETECTED_ARCH}.tar.gz)
      set(ROOT_SHA_KEY osx-xcode-13.1-${DETECTED_ARCH}-ROOT)
    endif()
  elseif("${DETECTED_OS_VERS}" MATCHES "^osx-11")
    message(FATAL_ERROR "We officially only support the latest macOS 11 versions 11.6, 11.7.")
  endif()
else()
  if("${DETECTED_OS_VERS}" MATCHES "^ubuntu-22")
    set(ROOT_TAR_FILE root_v6.28.02_python3.9_${DETECTED_OS_VERS}.tar.gz)
  endif()
endif()
set(ROOT_SHA ${${ROOT_SHA_KEY}})

message(STATUS "Using  ROOT tarball    : ${ROOT_TAR_FILE}")
message(STATUS "Using  ROOT source dir : ${ROOT_SOURCE_DIR}")
message(STATUS "Using  ROOT SHA key    : ${ROOT_SHA_KEY}")
message(STATUS "Verify ROOT SHA        : ${ROOT_SHA}")

download_verify_extract(
  http://cern.ch/biodynamo-lfs/third-party/${ROOT_TAR_FILE}
  ${ROOT_SOURCE_DIR}
  ${ROOT_SHA}
)

# Run again find_package in order to find ROOT
find_package(ROOT COMPONENTS Geom Gui GenVector REQUIRED)

# Set ROOTSYS variable
string(REGEX REPLACE "/include$" "" TMP_ROOT_PATH ${ROOT_INCLUDE_DIRS})
set(ENV{ROOTSYS} ${TMP_ROOT_PATH})

# Set ROOT_CONFIG_EXECUTABLE variable
find_program(ROOT_CONFIG_EXECUTABLE NAMES root-config HINTS "${TMP_ROOT_PATH}/bin")
SET(ROOT_CONFIG_EXECUTABLE ${ROOT_CONFIG_EXECUTABLE} PARENT_SCOPE)
