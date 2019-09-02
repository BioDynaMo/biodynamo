include(ExternalProject)

PRINT_LINE()
MESSAGE("Libroadrunner was not found in the system. Therefore it will be \
installed automatically now")
PRINT_LINE()

SET(LIB_RR_SOURCE_DIR "${CMAKE_THIRD_PARTY_DIR}/libroadrunner")
SET(LIB_RR_TAR_FILE "libroadrunner-11259a0.tar.gz")

file(DOWNLOAD http://cern.ch/biodynamo-lfs/third-party/${DETECTED_OS}/${LIB_RR_TAR_FILE}
        ${CMAKE_THIRD_PARTY_DIR}/${LIB_RR_TAR_FILE}
        EXPECTED_HASH SHA256=${${DETECTED_OS}-Libroadrunner}
        SHOW_PROGRESS)
file(MAKE_DIRECTORY ${LIB_RR_SOURCE_DIR})
execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf ${CMAKE_THIRD_PARTY_DIR}/${LIB_RR_TAR_FILE}
        WORKING_DIRECTORY ${LIB_RR_SOURCE_DIR})
