include(ExternalProject)

SET(QT_SOURCE_DIR "${CMAKE_THIRD_PARTY_DIR}")

file(DOWNLOAD http://cern.ch/biodynamo-lfs/third-party/${DETECTED_OS}/qt.tar.gz
        ${QT_SOURCE_DIR}/qt.tar.gz
        EXPECTED_HASH SHA256=${${DETECTED_OS}-Qt}
        SHOW_PROGRESS
        INACTIVITY_TIMEOUT 20
        STATUS QT_DOWNLOAD_STATUS)

# Check if the download worked properly.
LIST(GET QT_DOWNLOAD_STATUS 0 DOWNLOAD_STATUS)
IF (NOT ${DOWNLOAD_STATUS} EQUAL 0)
  MESSAGE( FATAL_ERROR "\nWe were unable to download Qt5. This may be caused by several reason, like \
network error connections or just temporary network failure. Please retry again in a \
few minutes by deleting all the contents of the build directory and by issuing again \
the 'cmake' command.\n")
ENDIF()


file(MAKE_DIRECTORY ${QT_SOURCE_DIR}/qt)
execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf ${QT_SOURCE_DIR}/qt.tar.gz
        WORKING_DIRECTORY ${QT_SOURCE_DIR}/qt)



# temporal workaround to avoid libprotobuf error for paraview
# use only until patched archive has been uploaded
IF (NOT APPLE)
    execute_process(COMMAND rm ${QT_SOURCE_DIR}/qt/plugins/platformthemes/libqgtk3.so
            WORKING_DIRECTORY ${QT_SOURCE_DIR}/qt)
    execute_process(COMMAND rm ${QT_SOURCE_DIR}/qt/lib/cmake/Qt5Gui/Qt5Gui_QGtk3ThemePlugin.cmake
            WORKING_DIRECTORY ${QT_SOURCE_DIR}/qt)
    execute_process(COMMAND touch ${QT_SOURCE_DIR}/qt/lib/cmake/Qt5Gui/Qt5Gui_QGtk3ThemePlugin.cmake
            WORKING_DIRECTORY ${QT_SOURCE_DIR}/qt)
ENDIF()
