include(utils)

SET(PARAVIEW_SOURCE_DIR "${CMAKE_THIRD_PARTY_DIR}/paraview")

if(APPLE)
  SET(PARAVIEW_TAR_FILE paraview_v5.10.0_${DETECTED_OS_VERS}_default.tar.gz)
else()
  SET(PARAVIEW_TAR_FILE paraview_v5.9.0_${DETECTED_OS_VERS}_default.tar.gz)
endif(APPLE)


download_verify_extract(
  http://cern.ch/biodynamo-lfs/third-party/${PARAVIEW_TAR_FILE}
  ${PARAVIEW_SOURCE_DIR}
  ${${DETECTED_OS_VERS}-ParaView}
)
