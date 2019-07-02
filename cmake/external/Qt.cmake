include(ExternalProject)

SET(QT_SOURCE_DIR "${CMAKE_THIRD_PARTY_DIR}/")

file(DOWNLOAD http://cern.ch/biodynamo-lfs/third-party/${DETECTED_OS}/qt.tar.gz
        ${QT_SOURCE_DIR}/qt.tar.gz
        EXPECTED_HASH SHA256=${${DETECTED_OS}-Qt}
        SHOW_PROGRESS)
file(MAKE_DIRECTORY ${QT_SOURCE_DIR}/qt)
execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf ${QT_SOURCE_DIR}/qt.tar.gz
        WORKING_DIRECTORY ${QT_SOURCE_DIR}/qt)
