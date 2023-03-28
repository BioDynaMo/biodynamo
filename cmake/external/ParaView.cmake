include(utils)

SET(PARAVIEW_SOURCE_DIR "${CMAKE_THIRD_PARTY_DIR}/paraview")

if(APPLE AND "${DETECTED_ARCH}" STREQUAL "i386")
  # The release of cmake 3.23.0 broke our build of ParaView on MacOSX. 
  # The build was fixed with a reupload and carries the additional tag cm233.
  SET(PARAVIEW_TAR_FILE paraview_v5.10.0_cm323_${DETECTED_OS_VERS}_default.tar.gz)
elseif(APPLE AND "${DETECTED_ARCH}" STREQUAL "arm64")
  SET(PARAVIEW_TAR_FILE paraview_v5.10.0_${DETECTED_OS_VERS}_default.tar.gz)
else()
  SET(PARAVIEW_TAR_FILE paraview_v5.9.0_${DETECTED_OS_VERS}_default.tar.gz)
endif()


download_verify_extract(
  http://cern.ch/biodynamo-lfs/third-party/${PARAVIEW_TAR_FILE}
  ${PARAVIEW_SOURCE_DIR}
  ${${DETECTED_OS_VERS}-ParaView}
)
