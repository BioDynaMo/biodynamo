include(utils)

SET(PARAVIEW_SOURCE_DIR "${CMAKE_THIRD_PARTY_DIR}/paraview")

# Which tarball belongs to this platform, and the key to verify it against.
# The mapping lives in cmake/utils.cmake so that the ParaView cache check in the
# top level CMakeLists.txt derives the same key.
bdm_paraview_platform(PARAVIEW_TAR_FILE PARAVIEW_SHA_KEY)
set(PARAVIEW_SHA ${${PARAVIEW_SHA_KEY}})

if("${PARAVIEW_SHA}" STREQUAL "")
  message(FATAL_ERROR "BioDynaMo does not ship a ParaView build for this platform "
    "(no SHA256 digest registered under the key '${PARAVIEW_SHA_KEY}'). "
    "Re-run cmake with -Dparaview=OFF to build without visualization support.")
endif()

message(STATUS "Using  ParaView tarball    : ${PARAVIEW_TAR_FILE}")
message(STATUS "Using  ParaView source dir : ${PARAVIEW_SOURCE_DIR}")
message(STATUS "Using  ParaView SHA key    : ${PARAVIEW_SHA_KEY}")
message(STATUS "Verify ParaView SHA        : ${PARAVIEW_SHA}")

download_verify_extract(
  http://cern.ch/biodynamo-lfs/third-party/${PARAVIEW_TAR_FILE}
  ${PARAVIEW_SOURCE_DIR}
  ${PARAVIEW_SHA}
)
