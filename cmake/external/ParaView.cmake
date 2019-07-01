include(ExternalProject)

SET(PARAVIEW_SOURCE_DIR "${CMAKE_THIRD_PARTY_DIR}/")

file(DOWNLOAD http://cern.ch/biodynamo-lfs/third-party/${DETECTED_OS}/paraview-v5.6.0.tar.gz
        ${PARAVIEW_SOURCE_DIR}/paraview-v5.6.0.tar.gz
        EXPECTED_MD5 04fc71bbd31dc4e604cfc7d88742b30a
        SHOW_PROGRESS)
file(MAKE_DIRECTORY ${PARAVIEW_SOURCE_DIR}/paraview)
execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf ${PARAVIEW_SOURCE_DIR}/paraview-v5.6.0.tar.gz
        WORKING_DIRECTORY ${PARAVIEW_SOURCE_DIR}/paraview)

