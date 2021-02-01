include(utils)

SET(PARAVIEW_SOURCE_DIR "${CMAKE_THIRD_PARTY_DIR}/paraview")

# Quickfix until the file on the CERN's repository is not renamed correctly
if (APPLE)
   EXECUTE_PROCESS(COMMAND sw_vers "-productVersion"
                   COMMAND cut -d . -f 1-2
                   OUTPUT_VARIABLE MACOS_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
   EXECUTE_PROCESS(COMMAND arch
                   OUTPUT_VARIABLE MACOS_ARCH OUTPUT_STRIP_TRAILING_WHITESPACE)
   SET(DETECTED_OS_VERS ${DETECTED_OS}-${MACOS_VERSION}-${MACOS_ARCH})
   SET(PARAVIEW_TAR_FILE paraview_v5.9.0_${DETECTED_OS_VERS}_default.tar.gz)
ELSE()
   SET(DETECTED_OS_VERS ${DETECTED_OS})
   SET(PARAVIEW_TAR_FILE ${DETECTED_OS_VERS}/paraview-v5.8.0.tar.gz)
ENDIF()

download_verify_extract(
  http://cern.ch/biodynamo-lfs/third-party/${PARAVIEW_TAR_FILE}
  ${PARAVIEW_SOURCE_DIR}
  ${${DETECTED_OS}-ParaView}
)
