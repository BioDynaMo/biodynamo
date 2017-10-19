# This file contains configuration for the install step

# set install directories
if(LINUX)
  set(CMAKE_INSTALL_BINDIR        "biodynamo/bin"                  CACHE PATH "User executables (bin)")
  set(CMAKE_INSTALL_INCLUDEDIR    "biodynamo/include"              CACHE PATH "C/C++ header files (include)")
  set(CMAKE_INSTALL_LIBDIR        "biodynamo/lib"                  CACHE PATH "Object code libraries (lib)")
  set(CMAKE_INSTALL_CMAKEDIR      "cmake"                          CACHE PATH "CMake files required from external projects")
  set(CMAKE_INSTALL_DATADIR       "biodynamo/share"                CACHE PATH "Read-only architecture-independent data (share)")
  set(CMAKE_INSTALL_CMAKEDATADIR  "${CMAKE_INSTALL_DATADIR}/cmake" CACHE PATH "Build related files (DATADIR/cmake)")
elseif(APPLE)
  set(CMAKE_INSTALL_BINDIR        "bin"                            CACHE PATH "User executables (bin)")
  set(CMAKE_INSTALL_INCLUDEDIR    "include/biodynamo"              CACHE PATH "C/C++ header files (include)")
  set(CMAKE_INSTALL_LIBDIR        "lib/biodynamo"                  CACHE PATH "Object code libraries (lib)")
  set(CMAKE_INSTALL_CMAKEDIR      "cmake"                          CACHE PATH "CMake files required from external projects")
  set(CMAKE_INSTALL_DATADIR       "share/biodynamo"                CACHE PATH "Read-only architecture-independent data (share)")
  set(CMAKE_INSTALL_CMAKEDATADIR  "${CMAKE_INSTALL_DATADIR}/cmake" CACHE PATH "Build related files (DATADIR/cmake)")
endif()

# hide them from configuration tools
mark_as_advanced(${CMAKE_INSTALL_BINDIR}
                 ${CMAKE_INSTALL_INCLUDEDIR}
                 ${CMAKE_INSTALL_LIBDIR}
                 ${CMAKE_INSTALL_CMAKEDIR}
                 ${CMAKE_INSTALL_DATADIR}
                 ${CMAKE_INSTALL_CMAKEDATADIR})

if(LINUX)
  install(FILES cmake/biodynamo_linux.env DESTINATION ${CMAKE_INSTALL_BINDIR} RENAME biodynamo.env)
  install(FILES cmake/get-biodynamo-env-path.sh DESTINATION ${CMAKE_INSTALL_BINDIR}
          PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
elseif(APPLE)
  install(FILES cmake/biodynamo_macos.env DESTINATION ${CMAKE_INSTALL_BINDIR} RENAME biodynamo.env)
endif()
# libbiodynamo.so
install(TARGETS biodynamo LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES build/libbiodynamo_dict_rdict.pcm DESTINATION ${CMAKE_INSTALL_LIBDIR})
# headers and python scripts
install(DIRECTORY src/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.h" PATTERN "*.py")
#   third party headers
file(GLOB MPARK_HEADERS ${CMAKE_BINARY_DIR}/mpark/mpark/*)
install(FILES ${MPARK_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/mpark)
install(FILES third_party/cpp_magic.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES third_party/OptionParser.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES third_party/cpptoml/cpptoml.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/cpptoml)
# build files
file(GLOB SELECTION_FILES cmake/*.xml)
install(FILES ${SELECTION_FILES} DESTINATION ${CMAKE_INSTALL_CMAKEDATADIR})
install(FILES cmake/BioDynaMo.cmake DESTINATION ${CMAKE_INSTALL_CMAKEDATADIR})
install(FILES cmake/SetCompilerFlags.cmake DESTINATION ${CMAKE_INSTALL_CMAKEDATADIR})
install(FILES cmake/FindROOT.cmake DESTINATION ${CMAKE_INSTALL_CMAKEDATADIR})
install(FILES cmake/RootUseFile.cmake DESTINATION ${CMAKE_INSTALL_CMAKEDATADIR})
install(FILES ${CMAKE_BINARY_DIR}/UseBioDynaMo.cmake DESTINATION ${CMAKE_INSTALL_CMAKEDATADIR})
# CMake files required from external projects
install(FILES cmake/BioDynaMoConfig.cmake DESTINATION ${CMAKE_INSTALL_CMAKEDIR})

# TODO copy of copyright information and distribution license
