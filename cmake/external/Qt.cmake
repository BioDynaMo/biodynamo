include(utils)

SET(QT_SOURCE_DIR "${CMAKE_THIRD_PARTY_DIR}/qt")

set(QT_TAR_FILE qt_v5.12.10_${DETECTED_OS_VERS}.tar.gz)

download_verify_extract(
  http://cern.ch/biodynamo-lfs/third-party/${DETECTED_OS}/qt.tar.gz
  ${QT_SOURCE_DIR}
  ${${DETECTED_OS_VERS}-Qt}
)

# temporal workaround to avoid libprotobuf error for paraview
# use only until patched archive has been uploaded
IF (NOT APPLE)
    execute_process(COMMAND rm ${QT_SOURCE_DIR}/plugins/platformthemes/libqgtk3.so
            WORKING_DIRECTORY ${QT_SOURCE_DIR})
    execute_process(COMMAND rm ${QT_SOURCE_DIR}/lib/cmake/Qt5Gui/Qt5Gui_QGtk3ThemePlugin.cmake
            WORKING_DIRECTORY ${QT_SOURCE_DIR})
    execute_process(COMMAND touch ${QT_SOURCE_DIR}/lib/cmake/Qt5Gui/Qt5Gui_QGtk3ThemePlugin.cmake
            WORKING_DIRECTORY ${QT_SOURCE_DIR})
ENDIF()
