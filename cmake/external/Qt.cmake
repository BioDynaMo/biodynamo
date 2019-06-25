include(ExternalProject)

SET(QT_SOURCE_DIR "${CMAKE_BINARY_DIR}/third_party/")

file(DOWNLOAD http://cern.ch/biodynamo-lfs/third-party/${DETECTED_OS}/qt.tar.gz ${QT_SOURCE_DIR}/qt.tar.gz
        SHOW_PROGRESS)
file(MAKE_DIRECTORY ${QT_SOURCE_DIR}/qt)
execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf ${QT_SOURCE_DIR}/qt.tar.gz
        WORKING_DIRECTORY ${QT_SOURCE_DIR}/qt)
#execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${QT_SOURCE_DIR}/qt ${THIRD_PARTY_DIR}/qt)
