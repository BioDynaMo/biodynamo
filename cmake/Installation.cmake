# This file contains configuration for the install step

# set install directories
set(CMAKE_INSTALL_BINDIR      "bin"                            CACHE PATH "user executables (bin)")
set(CMAKE_INSTALL_INCLUDEDIR  "include"                        CACHE PATH "C/C++ header files (include)")
set(CMAKE_INSTALL_LIBDIR      "lib"                            CACHE PATH "object code libraries (lib)")
set(CMAKE_INSTALL_DATADIR     "share"                          CACHE PATH "read-only architecture-independent data (share)")
set(CMAKE_INSTALL_CMAKEDIR    "${CMAKE_INSTALL_DATADIR}/cmake" CACHE PATH "Build related files (DATADIR/cmake)")

# hide them from configuration tools
mark_as_advanced(${CMAKE_INSTALL_BINDIR}
                 ${CMAKE_INSTALL_INCLUDEDIR}
                 ${CMAKE_INSTALL_LIBDIR}
                 ${CMAKE_INSTALL_DATADIR}
                 ${CMAKE_INSTALL_CMAKEDIR})

# libbiodynamo.so
install(TARGETS biodynamo LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
# headers
install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
#   third party headers
file(GLOB MPARK_HEADERS ${CMAKE_BINARY_DIR}/mpark/mpark/*)
install(FILES ${MPARK_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/mpark)
install(FILES third_party/cpp_magic.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES third_party/OptionParser.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
# build files
file(GLOB SELECTION_FILES cmake/*.xml)
install(FILES ${SELECTION_FILES} DESTINATION ${CMAKE_INSTALL_CMAKEDIR})
install(FILES cmake/SetCompilerFlags.cmake DESTINATION ${CMAKE_INSTALL_CMAKEDIR})
