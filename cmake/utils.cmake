# -----------------------------------------------------------------------------
#
# Copyright (C) The BioDynaMo Project.
# All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
#
# See the LICENSE file distributed with this work for details.
# See the NOTICE file distributed with this work for additional information
# regarding copyright ownership.
#
# -----------------------------------------------------------------------------

# Detect the system flavour and version. Generate a variable
# called BDM_OS which will have as content <OS>-<version>.
# If lsb_release is not found we ask the user to specify manually
# the software version.
function(detect_os)
    find_program(LSB_RELEASE_EXEC lsb_release)
    if (DETECTED_OS STREQUAL "none")
        if (DEFINED lsb_release-NOTFOUND AND NOT APPLE)
            MESSAGE(FATAL_ERROR "We were unable to detect the OS version. This happens because we did not find \
 the lsb_release command. In order to fix this error you should install the lsb_release command or specify which \
 system you are using. To specify the OS you have to run again cmake and set -DOS=<your_os> as argument. The current \
 supported OS'es are: ubuntu-18.04, ubuntu-20.04, centos-7, osx.")
        elseif(APPLE)
            SET(DETECTED_OS "osx" PARENT_SCOPE)
        else()
            execute_process(COMMAND ${LSB_RELEASE_EXEC} -is
                    OUTPUT_VARIABLE LSB_RELEASE_DISTRIBUTOR
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                    )
            execute_process(COMMAND ${LSB_RELEASE_EXEC} -sr
                    OUTPUT_VARIABLE LSB_RELEASE_RELEASE
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                    )
            if (${LSB_RELEASE_DISTRIBUTOR} STREQUAL "CentOS")
              string(SUBSTRING ${LSB_RELEASE_RELEASE} 0 1 CENTOS_MAJOR)
              SET(BDM_OS "${LSB_RELEASE_DISTRIBUTOR}-${CENTOS_MAJOR}")
            else()
              SET(BDM_OS "${LSB_RELEASE_DISTRIBUTOR}-${LSB_RELEASE_RELEASE}")
            endif()
            string(TOLOWER "${BDM_OS}" BDM_OS)
            SET(DETECTED_OS "${BDM_OS}" PARENT_SCOPE)
            SET(DETECTED_OS_VERSION ${LSB_RELEASE_RELEASE} PARENT_SCOPE)
            SET(DETECTED_OS_TYPE ${LSB_RELEASE_DISTRIBUTOR} PARENT_SCOPE)
        endif()
    endif()
endfunction()

# Try to find the ROOT package. It is an hard requirement
# for the project. If ROOT is not found in the system, it
# will be downloaded during the make step. Moreover, this
# will also check if ROOT was compiled using c++14 and if
# its environment was sourced.
function(verify_ROOT)
    if(NOT ROOT_FOUND)
        print_warning()
        message("We did not find any ROOT installed in the system. We will proceed to download it "
        "once the build process has started. ROOT will be then installed to the location ${CMAKE_THIRD_PARTY_DIR}/root.")
        print_line()
        include(external/ROOT)

        # Propagate the needed variables to the parent
        SET(ROOT_FOUND ${ROOT_FOUND} PARENT_SCOPE)
        SET(ROOT_VERSION ${ROOT_VERSION} PARENT_SCOPE)
        SET(ROOT_LIBRARIES ${ROOT_LIBRARIES} PARENT_SCOPE)
        SET(ROOT_LIBRARY_DIR ${ROOT_LIBRARY_DIR} PARENT_SCOPE)
        SET(ROOT_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} PARENT_SCOPE)
        SET(ROOT_ETC_DIR ${ROOT_ETC_DIR} PARENT_SCOPE)
        SET(ROOT_CONFIG_EXECUTABLE ${ROOT_CONFIG_EXECUTABLE} PARENT_SCOPE)
        SET(ROOTCLING_EXECUTABLE ${ROOTCLING_EXECUTABLE} PARENT_SCOPE)
        SET(GENREFLEX_EXECUTABLE ${GENREFLEX_EXECUTABLE} PARENT_SCOPE)
    else()
        # When ROOT is found, but it's not C++14 compliant, we exit the installation, because ROOT needs
        # to be properly sourced prior to invoking CMake (CMake cannot do this for us, because it requires
        # reverting the previous find_package() call, which is not possible.)
        if(NOT ROOT_cxx14_FOUND)
          message(FATAL_ERROR "The ROOT installation found in ${ROOTSYS} is not C++14 compliant. "
            "Please unset ROOTSYS and re-run cmake so that a compatible version of ROOT will be downloaded.")
        endif()

        if (NOT DEFINED ROOTSYS OR NOT DEFINED ${ROOTSYS})
          # Set ROOTSYS variable
          string(REGEX REPLACE "/include$" "" TMP_ROOT_PATH ${ROOT_INCLUDE_DIRS})
          set(ENV{ROOTSYS} ${TMP_ROOT_PATH})
        endif()
    endif()
endfunction()

# Convert a list to a better representation
# https://stackoverflow.com/questions/17666003/cmake-output-a-list-with-delimiters
function (ListToString result delim)
    list(GET ARGV 2 temp)
    math(EXPR N "${ARGC}-1")
    foreach(IDX RANGE 3 ${N})
        list(GET ARGV ${IDX} STR)
        set(temp "${temp}${delim}${STR}")
    endforeach()
    set(${result} "${temp}" PARENT_SCOPE)
endfunction(ListToString)

# Check if the OS given by the user is supported by the current BioDynaMo release.
#   OS: the OS specified by the user.
function(check_detected_os OS)
    # First of all we get a list of all the OS we are currently supporting
    FILE(GLOB SUPPORTED_OS_LIST LIST_DIRECTORIES TRUE ${CMAKE_SOURCE_DIR}/util/installation/*)

    # Retain only the name of the directory
    SET(SUPPORTED_OS "")
    foreach(VAR ${SUPPORTED_OS_LIST})
        get_filename_component(DIR ${VAR} NAME)
        LIST(APPEND SUPPORTED_OS ${DIR})
    endforeach()

    # Remove configuration names
    list(REMOVE_ITEM SUPPORTED_OS "common")

    # Pretty print the list
    ListToString(ALL_OS ", " ${SUPPORTED_OS})

    # Then we check if the given OS is in that list
    # If it is not then we issue a fatal error
    if (NOT "${OS}" IN_LIST SUPPORTED_OS)
        MESSAGE(FATAL_ERROR "The operating system you specified with the -DOS flag (${OS}) is not \
        supported by BioDynaMo. The operating systems we support at the moment are:
${ALL_OS}
")
    endif()
endfunction()

# This function will create a pre-install directory which will contain all the files
# needed to use BioDynaMo as an external project. Moreover, it will create custom
# targets such to run this procedure after the build process has completed.
function(install_inside_build)

    include(GreatCMakeCookOff/TargetCopyFiles)

    set(artifact_files_builddir)

    # Install the enviroment source script

    # Copy biodynamo.py and make it executable.
    add_custom_command(
            OUTPUT ${CMAKE_INSTALL_BINDIR}/biodynamo
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/cli/biodynamo.py ${CMAKE_INSTALL_BINDIR}/biodynamo
            DEPENDS ${CMAKE_SOURCE_DIR}/cli/biodynamo.py
    )
    list(APPEND artifact_files_builddir ${CMAKE_INSTALL_BINDIR}/biodynamo)

    # Copy header files
    add_copy_directory(copy_files_bdm
            ${CMAKE_SOURCE_DIR}/src/
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
            GLOB "*.h" "*.cl" "*.py"
            )

    # Copy cli files
    add_copy_directory(copy_files_bdm
            ${CMAKE_SOURCE_DIR}/cli/
            DESTINATION ${CMAKE_INSTALL_BINDIR}
            GLOB "*.py" "*.sh"
            EXCLUDE "biodynamo.py"
            )

    # Copy etc files
    add_copy_directory(copy_files_bdm
            ${CMAKE_SOURCE_DIR}/etc
            DESTINATION ${CMAKE_INSTALL_ROOT}/etc
            GLOB "*" ".*"
            )

    add_copy_files(copy_files_bdm
            ${CMAKE_BINARY_DIR}/rootlogon.C
            DESTINATION ${CMAKE_INSTALL_ROOT}/etc
            )

    add_copy_files(copy_files_bdm
            DESTINATION ${CMAKE_INSTALL_BINDIR}
            ${CMAKE_BINARY_DIR}/version/version.py
            ${CMAKE_SOURCE_DIR}/util/makefile-build/bdm-code-generation
            )

    # Copy some cmake files
    add_copy_files(copy_files_bdm
            DESTINATION ${CMAKE_INSTALL_CMAKEDATADIR}
            ${CMAKE_SOURCE_DIR}/cmake/BioDynaMoConfig.cmake
            ${CMAKE_SOURCE_DIR}/cmake/BioDynaMo.cmake
            ${CMAKE_SOURCE_DIR}/cmake/SetCompilerFlags.cmake
            ${CMAKE_SOURCE_DIR}/cmake/FindLibroadrunner.cmake
            ${CMAKE_SOURCE_DIR}/cmake/FindROOT.cmake
            ${CMAKE_SOURCE_DIR}/cmake/FindVTune.cmake
            ${CMAKE_SOURCE_DIR}/cmake/FindOpenCL.cmake
            ${CMAKE_SOURCE_DIR}/cmake/FindNuma.cmake
            ${CMAKE_SOURCE_DIR}/cmake/FindClangTools.cmake
            ${CMAKE_SOURCE_DIR}/cmake/Findtcmalloc.cmake
            ${CMAKE_SOURCE_DIR}/cmake/Findjemalloc.cmake
            ${CMAKE_SOURCE_DIR}/cmake/RootUseFile.cmake
            ${CMAKE_SOURCE_DIR}/cmake/CppStyleGuideChecks.cmake
            ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/UseBioDynaMo.cmake
            ${CMAKE_SOURCE_DIR}/cmake/utils.cmake
            )
    add_copy_files(copy_files_bdm
            DESTINATION ${CMAKE_INSTALL_CMAKEDATADIR}/../util
            GLOB ${CMAKE_SOURCE_DIR}/util/housekeeping/*.sh
            )
    add_copy_files(copy_files_bdm
            DESTINATION ${CMAKE_INSTALL_CMAKEDATADIR}/../util/support_files
            ${CMAKE_SOURCE_DIR}/.clang-format
            ${CMAKE_SOURCE_DIR}/.clang-tidy
            ${CMAKE_SOURCE_DIR}/.clang-tidy-ignore
            ${CMAKE_SOURCE_DIR}/.gitignore
            )

    add_copy_directory(copy_files_bdm
            ${CMAKE_SOURCE_DIR}/third_party/cpplint
            DESTINATION ${CMAKE_INSTALL_ROOT}/third_party/cpplint
            GLOB "*")

    add_copy_files(copy_files_bdm
            DESTINATION ${CMAKE_INSTALL_CMAKEDATADIR}
            GLOB ${CMAKE_SOURCE_DIR}/cmake/*.xml
            )

    # Other headers
    add_copy_files(copy_files_bdm
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
            ${CMAKE_BINARY_DIR}/version/version.h
            )

    # libbdmcuda.a
    if(CUDA_FOUND)
        install(TARGETS bdmcuda ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} OPTIONAL)
    endif()

    #third party headers
    add_copy_directory(copy_files_bdm
            ${CMAKE_CURRENT_BINARY_DIR}/extracted-third-party-libs/morton
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/morton
            GLOB "*"
            )
    add_copy_directory(copy_files_bdm
            ${CMAKE_CURRENT_BINARY_DIR}/extracted-third-party-libs/mpark
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/mpark
            GLOB "*"
            )
    add_copy_files(copy_files_bdm
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
            ${CMAKE_SOURCE_DIR}/third_party/cpp_magic.h
            ${CMAKE_SOURCE_DIR}/third_party/cxxopts-v2.2.0/cxxopts.h
            )
    add_copy_files(copy_files_bdm
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/cpptoml
            ${CMAKE_SOURCE_DIR}/third_party/cpptoml/cpptoml.h
            )

    # Simulation and demos
    add_copy_directory(copy_files_bdm
            ${CMAKE_SOURCE_DIR}/util/simulation-template
            DESTINATION ${CMAKE_INSTALL_ROOT}/simulation-template
            GLOB "*"
            )
    add_copy_directory(copy_files_bdm
            ${CMAKE_SOURCE_DIR}/demo
            DESTINATION ${CMAKE_INSTALL_ROOT}/demo
            GLOB "*"
            EXCLUDE "build*"
            )

    # Copy legal stuff
    add_copy_files(copy_files_bdm
            DESTINATION ${CMAKE_INSTALL_ROOT}
            ${CMAKE_SOURCE_DIR}/LICENSE
            ${CMAKE_SOURCE_DIR}/NOTICE
            )

    # BioDynaMo paraview plugin (Apple support when we upgrade to v5.8 on macos)
    if(paraview AND NOT APPLE)
      add_copy_files(copy_files_bdm
              DESTINATION ${CMAKE_INSTALL_PVPLUGINDIR}
              ${CMAKE_INSTALL_LIBDIR}/paraview-5.8/plugins/BDMGlyphFilter/BDMGlyphFilter.so
              )
      add_copy_files(copy_files_bdm
              DESTINATION ${CMAKE_INSTALL_ROOT}/lib
              ${CMAKE_INSTALL_LIBDIR}/paraview-5.8/plugins/BDMGlyphFilter/libBDM.so
              )
    endif()

    if (test)
      message("-------------------------")
      message(              ${CMAKE_SOURCE_DIR}/test/unit)
      add_copy_directory(copy_files_bdm
              ${CMAKE_SOURCE_DIR}/test/unit/
              DESTINATION ${CMAKE_INSTALL_ROOT}/share/test
              GLOB "**/*.py"
              )
    endif()

    add_custom_target(copy_files_bdm ALL DEPENDS ${artifact_files_builddir})
    add_dependencies(copy_files_bdm biodynamo)
    if(paraview)
      add_dependencies(copy_files_bdm BDMGlyphFilter)
    endif()

endfunction()

# This function add a description to the packages which will be displayed
# at the end of the cmake run.
function(add_bdm_packages_properties)
    SET_PACKAGE_PROPERTIES(MPI PROPERTIES
            DESCRIPTION "OpenMPI, an Open Source Message Passing Interface. (REQUIRED)"
            TYPE REQUIRED
            )
    SET_PACKAGE_PROPERTIES(GLUT PROPERTIES
            DESCRIPTION "Open Source alternative to the OpenGL Utility Toolkit (GLUT) library. (OPTIONAL)"
            TYPE REQUIRED
            )
    SET_PACKAGE_PROPERTIES(OpenMP PROPERTIES
            DESCRIPTION "API that enables multi-platform shared memory multiprocessing programming. (REQUIRED)"
            TYPE REQUIRED
            )
    SET_PACKAGE_PROPERTIES(Threads PROPERTIES
            DESCRIPTION "GNU C library POSIX threads implementation. (REQUIRED)"
            TYPE REQUIRED
            )
    SET_PACKAGE_PROPERTIES(OpenCL PROPERTIES
            DESCRIPTION "Code generation for GPU acceleration. (OPTIONAL)"
            TYPE REQUIRED
            )
    SET_PACKAGE_PROPERTIES(Git PROPERTIES
            DESCRIPTION "Open Source Distributed Version Control System. (OPTIONAL)"
            TYPE REQUIRED
            )
    SET_PACKAGE_PROPERTIES(ROOT PROPERTIES
            DESCRIPTION "CERN's Modular Scientific Software Toolkit. (REQUIRED)"
            TYPE REQUIRED
            )
    SET_PACKAGE_PROPERTIES(ClangTools PROPERTIES
            DESCRIPTION "Standalone command line tools that provide developer-oriented functionalities. (OPTIONAL)"
            TYPE REQUIRED
            )
    SET_PACKAGE_PROPERTIES(Numa PROPERTIES
            DESCRIPTION "Simple API to the NUMA (Non Uniform Memory Access) policy supported by the Linux kernel. (REQUIRED only on Linux system)"
            TYPE REQUIRED
            )
    SET_PACKAGE_PROPERTIES(ParaView PROPERTIES
            DESCRIPTION "Open Source, multi-platform data analysis and visualization application. (OPTIONAL)"
            TYPE REQUIRED
            )
    SET_PACKAGE_PROPERTIES(Valgrind PROPERTIES
            DESCRIPTION "A suite of tools for debugging and profiling. (OPTIONAL)"
            TYPE REQUIRED
            )
    SET_PACKAGE_PROPERTIES(Doxygen PROPERTIES
            DESCRIPTION "Tool for generating documentation from annotated C++ sources. (OPTIONAL)"
            TYPE REQUIRED
            )
    SET_PACKAGE_PROPERTIES(MKDocs PROPERTIES
            DESCRIPTION "Fast, simple and downright gorgeous static site generator that's geared towards building project documentation. (OPTIONAL)"
            TYPE REQUIRED
            )
    SET_PACKAGE_PROPERTIES(PythonInterp PROPERTIES
            DESCRIPTION "Python executable. (REQUIRED)"
            TYPE REQUIRED
            )
    SET_PACKAGE_PROPERTIES(pip PROPERTIES
            DESCRIPTION "Package Manager System for Python. (REQUIRED)"
            TYPE REQUIRED
            )
    SET_PACKAGE_PROPERTIES(Qt5 PROPERTIES
            DESCRIPTION "Open Source widget toolkit for creating user interfaces. It is needed by Paraview. (OPTIONAL)"
            TYPE REQUIRED
            )

endfunction()

# Add a small description to the -D flags which we can use
# when running cmake. It is used to show the final user more
# information about the build system.
function(add_bdm_feature_properties)
    ADD_FEATURE_INFO(test test "Build BioDynaMo's test suite.")
    ADD_FEATURE_INFO(cuda cuda "Enable CUDA code generation for GPU acceleration.")
    ADD_FEATURE_INFO(opencl opencl "Enable OpenCL code generation for GPU acceleration.")
    ADD_FEATURE_INFO(dict dict "Build with ROOT dictionaries.")
    ADD_FEATURE_INFO(numa numa "Enable NUMA-Awareness in BioDynaMo.")
    ADD_FEATURE_INFO(paraview paraview "Enable ParaView.")
    ADD_FEATURE_INFO(sbml sbml "Enable SBML integration.")
    ADD_FEATURE_INFO(vtune vtune "Enable VTune performance analysis.")
    ADD_FEATURE_INFO(coverage coverage "Enable test coverage report generation. Sets build type to coverage.")
    ADD_FEATURE_INFO(verbose verbose "Enable verbosity when running make install.")
    ADD_FEATURE_INFO(tcmalloc tcmalloc "Use tcmalloc for memory allocations.")
    ADD_FEATURE_INFO(jemalloc jemalloc "Use jemalloc for memory allocations.")
    ADD_FEATURE_INFO(notebooks notebooks "Generate ROOT notebooks")
endfunction()

# Method used to give execution permissions to a file
#   FILE_PATH: the path to the file we want to update;
#   DESTINATION: location where we want to copy the file.
function(add_permissions FILE_PATH DESTINATION)
    file(COPY ${FILE_PATH}
            DESTINATION ${DESTINATION}
            FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ
            GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
endfunction()

# Method used to download a file from a given URL. This method will also retry
# to download the file if the download did not work.
#   URL: URL from which we will download the file
#   DEST: destination where the contents of the tar file will be extracted to
#   HASH: hash of the download file to check consistency
function(download_verify_extract URL DEST HASH)
  file(MAKE_DIRECTORY ${DEST})
  get_filename_component(TAR_FILENAME ${URL} NAME)
  get_filename_component(DEST_PARENT "${DEST}/.." ABSOLUTE)
  set(FULL_TAR_PATH "${DEST_PARENT}/${TAR_FILENAME}")

  # Download the file
  execute_process(COMMAND ${WGET_BIN} --progress=dot:giga -O ${FULL_TAR_PATH} ${URL}
                  RESULT_VARIABLE DOWNLOAD_STATUS_CODE)
  if (NOT ${DOWNLOAD_STATUS_CODE} EQUAL 0)
    message( FATAL_ERROR "\nERROR: We were unable to download:\
  ${URL}\n\
This may be caused by several reason, like network error connections or just \
temporary network failure. Please retry again in a few minutes by deleting all \
the contents of the build directory and by issuing again the 'cmake' command.\n")
  endif()

  # Verify download
  file(SHA256 ${FULL_TAR_PATH} ACTUAL_SHA256)
  if(NOT ACTUAL_SHA256 STREQUAL "${HASH}")
    message(FATAL_ERROR "\nERROR: SHA256 sum verification failed.\n\
  Expected: ${HASH}\n\
  Actual:   ${ACTUAL_SHA256}\n")
  endif()

  # Extract
  execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf ${FULL_TAR_PATH}
                  WORKING_DIRECTORY ${DEST}
                RESULT_VARIABLE EXTRACT_STATUS_CODE)
  if (NOT ${EXTRACT_STATUS_CODE} EQUAL 0)
    message(FATAL_ERROR "ERROR: Extraction of file ${FULL_TAR_PATH} to ${DEST} failed.")
  endif()

  # remove tar file
  file(REMOVE ${FULL_TAR_PATH})
endfunction()

# Helper function to print a simple line
function(print_line)
    MESSAGE("\n################################################################\n")
endfunction()

# Helper function to print a warning message
function(print_warning)
    MESSAGE("\n########################### WARNING ############################\n")
endfunction()

# Helper function to print a summary indication
function(print_summary)
    MESSAGE("\n########################### SUMMARY ############################\n")
endfunction()

function(filter_list OUTPUT INPUT REGEX)
  set(ITEMS)
  foreach(ITR ${INPUT})
    if(NOT ITR MATCHES ${REGEX})
      list(APPEND ITEMS ${ITR})
    endif()
  endforeach()
  set(${OUTPUT} ${ITEMS} PARENT_SCOPE)
endfunction()
