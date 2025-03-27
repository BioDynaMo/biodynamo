include(utils)


SET(QT_SOURCE_DIR "${CMAKE_THIRD_PARTY_DIR}/qt")
SET(QT_BLD_SCRIPT_LOCATION "${CMAKE_SOURCE_DIR}/util/build-third-party")
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/build/third_party")
		file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/build/third_party")
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/qt")
			file(COPY "${CMAKE_SOURCE_DIR}/third_party/qt" DESTINATION "${CMAKE_SOURCE_DIR}/build/third_party")
else()
	execute_process(COMMAND bash ${QT_BLD_SCRIPT_LOCATION}/build-qt_new.sh ${CMAKE_SOURCE_DIR} 		RESULT_VARIABLE QT_DOWNLOAD_OR_BUILD)



	if(${QT_DOWNLOAD_OR_BUILD} EQUAL 1)
		set(QT_TAR_FILE qt_v5.12.10_${DETECTED_OS_VERS}.tar.gz)

		download_verify_extract(
 	 	http://cern.ch/biodynamo-lfs/third-party/${QT_TAR_FILE}
 	 	${QT_SOURCE_DIR}
  	 	${${DETECTED_OS_VERS}-Qt}
		)
	elseif(${QT_DOWNLOAD_OR_BUILD} EQUAL 0)
		file(COPY "${CMAKE_SOURCE_DIR}/third_party/qt" DESTINATION "${CMAKE_SOURCE_DIR}/build/third_party")
	
	endif()
endif()
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
