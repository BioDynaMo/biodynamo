include(ExternalProject)

PRINT_LINE()
MESSAGE("Libroadrunner was not found in the system. Therefore it will be \
installed automatically now")
PRINT_LINE()

SET(LIB_RR_SOURCE_DIR "${CMAKE_THIRD_PARTY_DIR}/libroadrunner")
SET(LIB_RR_TAR_FILE "libroadrunner-3cbfbedba.tar.gz")

download_verify_extract(
  http://cern.ch/biodynamo-lfs/third-party/${DETECTED_OS}/${LIB_RR_TAR_FILE}
  ${LIB_RR_SOURCE_DIR}
  ${${DETECTED_OS}-Libroadrunner}
)
