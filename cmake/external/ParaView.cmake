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
set(PARAVIEW_SHA_KEY ${DETECTED_OS_VERS}-ParaView)
set(PARAVIEW_SHA ${${PARAVIEW_SHA_KEY}})

message(STATUS "Using  ParaView tarball    : ${PARAVIEW_TAR_FILE}")
message(STATUS "Using  ParaView source dir : ${PARAVIEW_SOURCE_DIR}")
message(STATUS "Using  ParaView SHA key    : ${PARAVIEW_SHA_KEY}")
message(STATUS "Verify ParaView SHA        : ${PARAVIEW_SHA}")

download_verify_extract(
  http://cern.ch/biodynamo-lfs/third-party/${PARAVIEW_TAR_FILE}
  ${PARAVIEW_SOURCE_DIR}
  ${PARAVIEW_SHA}
)