include(utils)

SET(PARAVIEW_SOURCE_DIR "${CMAKE_THIRD_PARTY_DIR}/paraview")

# Quickfix until the file on the CERN's repository is not renamed correctly
if (APPLE)
    SET(PARAVIEW_DOWNLOAD_NAME paraview.tar.gz)
ELSE()
    SET(PARAVIEW_DOWNLOAD_NAME paraview-v5.6.0.tar.gz)
ENDIF()

download_verify_extract(
  http://cern.ch/biodynamo-lfs/third-party/${DETECTED_OS}/${PARAVIEW_DOWNLOAD_NAME}
  ${PARAVIEW_SOURCE_DIR}
  ${${DETECTED_OS}-ParaView}
)
