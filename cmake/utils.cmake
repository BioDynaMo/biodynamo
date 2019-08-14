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

# Try to find the ROOT package. It is an hard requirement
# for the project. If ROOT is not found in the system, it
# will be downloaded during the make step. Moreover, this
# will also check if ROOT was compiled using c++14 and if
# its environment was sourced.
function(verify_ROOT)
    set(CUSTOM_ROOT_SOURCE_ENV FALSE PARENT_SCOPE)
    set(CUSTOM_ROOT_SOURCE_ENV_COMMAND ":" PARENT_SCOPE)
    IF(NOT ROOT_FOUND)
        print_warning()
        MESSAGE("We did not find any ROOT installed in the system. We will proceed to download it\n\
once the build process has started. ROOT will be then installed to the location ${THIRD_PARTY_DIR}/root.")
        print_line()
        include(external/ROOT)

        # Propagate the needed variables to the parent
        SET(ROOT_LIBRARIES ${ROOT_LIBRARIES} PARENT_SCOPE)
        SET(ROOT_INCLUDES ${ROOT_INCLUDES} PARENT_SCOPE)
        SET(ROOT_LIBRARY_DIR ${ROOT_LIBRARY_DIR} PARENT_SCOPE)
        SET(ROOT_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} PARENT_SCOPE)
    else()
        # Check if ROOT was compiled with the correct C++ standard (which has to be greater or equal
        # than C++14). If that's not the case, then we will download the correct version and we will
        # use that instead of the system one.
        if (NOT ROOT_CXX_FLAGS MATCHES ["-std=c++14"|"-std=c++1y"])
            MESSAGE(WARNING "The ROOT version currently installed was compiled with an older c++ standard that is not\
compatible with BioDynaMo. We will proceed to download the correct version of ROOT now.")
            include(external/ROOT)

            # Propagate the needed variables to the parent
            SET(ROOT_LIBRARIES ${ROOT_LIBRARIES} PARENT_SCOPE)
            SET(ROOT_INCLUDES ${ROOT_INCLUDES} PARENT_SCOPE)
            SET(ROOT_LIBRARY_DIR ${ROOT_LIBRARY_DIR} PARENT_SCOPE)
            SET(ROOT_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} PARENT_SCOPE)
        endif()

        # Manually set the ROOT enviromental variables if it ROOTSYS was not previously found.
        # This will avoid the user to run thisroot.sh in order to build BioDynaMo. However,
        # we will warn him anyway, such to let him/her know that this is not exactly the
        # standard way to do things.
        if (NOT DEFINED ENV{ROOTSYS})
            string(REGEX REPLACE "/include$" "" TMP_ROOT_PATH ${ROOT_INCLUDE_DIRS})
            set(ENV{ROOTSYS} ${TMP_ROOT_PATH} PARENT_SCOPE)
            set(ENV{LD_LIBRARY_PATH} $ENV{ROOTSYS}/lib:$ENV{LD_LIBRARY_PATH} PARENT_SCOPE)

            # Since the user did not set the $ROOTSYS variable, then we need to source
            # the ROOT environment everytime we need it. Ideally, when ROOT env is needed
            # one should use the command outlined before in its custom targets.
            source_root_file(${PROJECT_BINARY_DIR})
            set(CUSTOM_ROOT_SOURCE_ENV TRUE PARENT_SCOPE)
            set(CUSTOM_ROOT_SOURCE_ENV_COMMAND . ${PROJECT_BINARY_DIR}/source_root_auto.sh PARENT_SCOPE)

            PRINT_WARNING()
            MESSAGE("It appears that ROOT environment was not loaded
(by sourcing ${ROOTSYS}/bin/thisroot.sh).
You will be able to build BioDynaMo anyway.")
            PRINT_LINE()
        endif()
    endif()
endfunction()

# Detect the system flavour and version. Generate a variable
# called BDM_OS which will have as content <OS>-<version>.
# If lsb_release is not found we ask the user to specify manually
# the software version. Moreover, another check is performed such to
# detect if we are building BioDynaMo for the CI (Travis).
function(detect_os)
    find_program(LSB_RELEASE_EXEC lsb_release)
    if (DETECTED_OS STREQUAL "none")
        if (DEFINED lsb_release-NOTFOUND AND NOT APPLE AND NOT DEFINED ${DETECTED_OS})
            MESSAGE(FATAL_ERROR "We were unable to detect your OS version. This usually happens because we were unable to find\
 lsb_release command. In order to fix this error you can: install lsb_release for your distribution or specify which \
 system you are using. To specify the OS you have to call again cmake by passing -DOS=<your_os> as argument. The current\
 supported OS are: ubuntu-16.04, ubuntu-18.04, centos-7.6.1810, osx.")
        elseif(APPLE)
            # We check if we are using Travis (therefore the BDM_OS has a slightly different name).
            if ($ENV{TRAVIS})
                SET(DETECTED_OS "travis-osx" PARENT_SCOPE)
            else()
                SET(DETECTED_OS "osx" PARENT_SCOPE)
            endif()
        else()
            execute_process(COMMAND ${LSB_RELEASE_EXEC} -is
                    OUTPUT_VARIABLE LSB_RELEASE_ID_SHORT
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                    )
            execute_process(COMMAND ${LSB_RELEASE_EXEC} -sr
                    OUTPUT_VARIABLE LSB_RELEASE_ID_VERSION
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                    )
            SET(BDM_OS "${LSB_RELEASE_ID_SHORT}-${LSB_RELEASE_ID_VERSION}")
            string(TOLOWER "${BDM_OS}" BDM_OS)
            SET(DETECTED_OS "${BDM_OS}" PARENT_SCOPE)
            SET(DETECTED_OS_VERSION ${LSB_RELEASE_ID_VERSION} PARENT_SCOPE)
            SET(DETECTED_OS_TYPE ${LSB_RELEASE_ID_SHORT} PARENT_SCOPE)
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
        MESSAGE(FATAL_ERROR "The operative system you specified with the -DOS flag (${OS}) is not\
        supported by the current BioDynaMo. The operative systems we support at the moment are:
${ALL_OS}
")
    endif()
endfunction()

# Copy the source_root_auto.sh file by supplying the right information
function(source_root_file INSTALL_DIR)
    configure_file(cmake/env/source_root_auto.sh ${INSTALL_DIR}/source_root_auto.sh @ONLY)
endfunction()

# This function will create a pre-install directory which will contain all the files
# needed to use BioDynaMo as an external project. Moreover, it will create custom
# targets such to run this procedure after the build process has completed.
function(install_inside_build)

    include(GreatCMakeCookOff/TargetCopyFiles)

    add_custom_target(copy_files_bdm ALL DEPENDS biodynamo)

    # Install the enviroment source script

    # Copy biodynamo.py and make it executable.
    add_copy_files(copy_files_bdm
            DESTINATION ${CMAKE_INSTALL_BINDIR}
            ${CMAKE_SOURCE_DIR}/cli/biodynamo.py)
    add_custom_command(
            TARGET copy_files_bdm
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E rename ${CMAKE_INSTALL_BINDIR}/biodynamo.py ${CMAKE_INSTALL_BINDIR}/biodynamo
            VERBATIM
            DEPENDS ${CMAKE_SOURCE_DIR}/cli/biodynamo.py
    )

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
            GLOB "*.py"
            EXCLUDE "biodynamo.py"
            )

    add_copy_files(copy_files_bdm
            DESTINATION ${CMAKE_INSTALL_BINDIR}
            ${CMAKE_BINARY_DIR}/version/version.py
            ${CMAKE_SOURCE_DIR}/util/makefile-build/bdm-config
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
            ${CMAKE_SOURCE_DIR}/cmake/FindTBB.cmake
            ${CMAKE_SOURCE_DIR}/cmake/FindClangTools.cmake
            ${CMAKE_SOURCE_DIR}/cmake/RootUseFile.cmake
            ${CMAKE_SOURCE_DIR}/cmake/CppStyleGuideChecks.cmake
            ${CMAKE_BINARY_DIR}/UseBioDynaMo.cmake
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
            ${CMAKE_SOURCE_DIR}/third_party/OptionParser.h
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
            DESCRIPTION "Simple API to the NUMA (Non Uniform Memory Access) policy supported by the Linux kernel. (REQUIRED)"
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
    ADD_FEATURE_INFO(paraview paraview "Enable ParaView.")
    ADD_FEATURE_INFO(sbml sbml "Enable SBML integration.")
    ADD_FEATURE_INFO(vtune vtune "Enable VTune performance analysis.")
    ADD_FEATURE_INFO(coverage coverage "Enable test coverage report generation. Sets build type to coverage.")
    ADD_FEATURE_INFO(verbose verbose "Enable verbosity when running make install.")
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
