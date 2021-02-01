include(utils)

SET(QT_SOURCE_DIR "${CMAKE_THIRD_PARTY_DIR}/qt")

IF (APPLE)
   EXECUTE_PROCESS(COMMAND sw_vers "-productVersion"
                   COMMAND cut -d . -f 1-2
                   OUTPUT_VARIABLE MACOS_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
   EXECUTE_PROCESS(COMMAND arch
                   OUTPUT_VARIABLE MACOS_ARCH OUTPUT_STRIP_TRAILING_WHITESPACE)
   set(DETECTED_OS_VERS ${DETECTED_OS}-${MACOS_VERSION}-${MACOS_ARCH})
   set(QT_TAR_FILE qt_v5.12.10_${DETECTED_OS_VERS}.tar.gz)
ELSE()
   set(DETECTED_OS_VERS ${DETECTED_OS})
   set(QT_TAR_FILE ${DETECTED_OS_VERS}/qt.tar.gz)
ENDIF()

download_verify_extract(
  http://cern.ch/biodynamo-lfs/third-party/${QT_TAR_FILE}
  ${QT_SOURCE_DIR}
  ${${DETECTED_OS}-Qt}
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
