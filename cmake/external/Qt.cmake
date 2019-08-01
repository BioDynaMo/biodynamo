include(ExternalProject)

SET(QT_SOURCE_DIR "${CMAKE_THIRD_PARTY_DIR}/")

file(DOWNLOAD http://cern.ch/biodynamo-lfs/third-party/${DETECTED_OS}/qt.tar.gz
        ${QT_SOURCE_DIR}/qt.tar.gz
        EXPECTED_MD5 6dd383814c3275f2b07db4c9661115bf
        SHOW_PROGRESS)
file(MAKE_DIRECTORY ${QT_SOURCE_DIR}/qt)
execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf ${QT_SOURCE_DIR}/qt.tar.gz
        WORKING_DIRECTORY ${QT_SOURCE_DIR}/qt)
