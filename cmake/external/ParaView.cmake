include(ExternalProject)

SET(PARAVIEW_SOURCE_DIR "${CMAKE_BINARY_DIR}/third_party/")

file(DOWNLOAD http://cern.ch/biodynamo-lfs/third-party/${DETECTED_OS}/paraview-v5.6.0.tar.gz ${PARAVIEW_SOURCE_DIR}/paraview-v5.6.0.tar.gz
        SHOW_PROGRESS)
file(MAKE_DIRECTORY ${PARAVIEW_SOURCE_DIR}/paraview)
execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf ${PARAVIEW_SOURCE_DIR}/paraview-v5.6.0.tar.gz
        WORKING_DIRECTORY ${PARAVIEW_SOURCE_DIR}/paraview)
#execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${PARAVIEW_SOURCE_DIR}/paraview ${THIRD_PARTY_DIR}/paraview)

