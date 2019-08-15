include(utils)

SET(QT_SOURCE_DIR "${CMAKE_THIRD_PARTY_DIR}")

# Download the package and retry if something went wrong
download_retry(
  http://cern.ch/biodynamo-lfs/third-party/${DETECTED_OS}/qt.tar.gz
  ${QT_SOURCE_DIR}/qt.tar.gz
  ${${DETECTED_OS}-Qt}
  "Qt5"
)

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
