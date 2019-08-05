include(ExternalProject)

SET(PARAVIEW_SOURCE_DIR "${CMAKE_THIRD_PARTY_DIR}/")

# Quickfix until the file on the CERN's repository is not renamed correctly
if (APPLE)
    SET(PARAVIEW_DOWNLOAD_NAME paraview.tar.gz)
ELSE()
    SET(PARAVIEW_DOWNLOAD_NAME paraview-v5.6.0.tar.gz)
ENDIF()

file(DOWNLOAD http://cern.ch/biodynamo-lfs/third-party/${DETECTED_OS}/${PARAVIEW_DOWNLOAD_NAME}
        ${PARAVIEW_SOURCE_DIR}/paraview-v5.6.0.tar.gz
        EXPECTED_HASH SHA256=${${DETECTED_OS}-ParaView}
        SHOW_PROGRESS)
file(MAKE_DIRECTORY ${PARAVIEW_SOURCE_DIR}/paraview)
execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf ${PARAVIEW_SOURCE_DIR}/paraview-v5.6.0.tar.gz
        WORKING_DIRECTORY ${PARAVIEW_SOURCE_DIR}/paraview)

