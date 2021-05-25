include(utils)

SET(PARAVIEW_SOURCE_DIR "${CMAKE_THIRD_PARTY_DIR}/paraview")

SET(PARAVIEW_TAR_FILE paraview_v5.9.0_${DETECTED_OS_VERS}_default.tar.gz)

download_verify_extract(
  http://cern.ch/biodynamo-lfs/third-party/${DETECTED_OS}/paraview-v5.8.0.tar.gz
  ${PARAVIEW_SOURCE_DIR}
  ${${DETECTED_OS_VERS}-ParaView}
)
