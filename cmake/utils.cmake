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
        MESSAGE("We did not found any ROOT installed in the system. We will proceed to download it\n\
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
            MESSAGE("It appears that ROOT environment was not loaded (by sourcing ${ROOTSYS}/bin/thisroot.sh).\n\
You will be able to build BioDynaMo anyway. However, make sure to source it later\n on in order to use the library.")
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
        if (DEFINED lsb_release-NOTFOUND AND NOT APPLE AND NOT DEFINED ${BDM_OS})
            MESSAGE(FATAL_ERROR "We were unable to detect your OS version. This usually happens because we were unable to find\
 lsb_release command. In order to fix this error you can: install lsb_release for your distribution or specify which \
 system you are using. To specify the OS you have to call again cmake by passing -DBDM_OS=<your_os> as argument. The current\
 supported OS are: ubutu-16.04, ubuntu-18.04, centos, osx.")
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
        endif()
    endif()
endfunction()

function(source_root_file INSTALL_DIR)
    configure_file(cmake/env/source_root_auto.sh ${INSTALL_DIR}/source_root_auto.sh @ONLY)
endfunction()


# This function will generated bash scripts to download the missing requirements
# (both optional and mandatory) needed to build correctly BioDynaMo. It will also
# print some information to the user to instruct him/her about how to use them.
#
# * OS: it represents the current OS version. It can be detected using the
#       previous function called detect_os();
# * MISSING_PACKAGES: a list of the missing packages;
# * REQUIRED_PACKAGES; a list of required packages;
function(generate_download_prerequisites OS MISSING_PACKAGES REQUIRED_PACKAGES)

    # We remove some packages they are installed automatically
    list(REMOVE_ITEM MISSING_PACKAGES "ParaView" "ROOT" "GCCXML")
    list(LENGTH MISSING_PACKAGES MISSING_PACKAGES_SIZE)

    # Default optional package variable
    SET(OPTIONAL_PACKAGES "${MISSING_PACKAGES}")
    list(LENGTH OPTIONAL_PACKAGES OPTIONAL_PACKAGES_SIZE)

    IF(NOT ${MISSING_PACKAGES_SIZE} EQUAL "0")
        # We first get the required packages we are missing
        list(REMOVE_ITEM OPTIONAL_PACKAGES ${REQUIRED_PACKAGES})
        list(REMOVE_ITEM MISSING_PACKAGES ${OPTIONAL_PACKAGES})

        # Compute how many required and optional packages we are missing
        list(LENGTH MISSING_PACKAGES MISSING_PACKAGES_SIZE)
        list(LENGTH OPTIONAL_PACKAGES OPTIONAL_PACKAGES_SIZE)

        # Include the prerequisites for the given OS and initialize the setup files
        include(prerequisites/${OS}-install)
        file(WRITE ${CMAKE_BINARY_DIR}/prerequisites-required.sh "${OS_SHEBANG}\n")
        file(WRITE ${CMAKE_BINARY_DIR}/prerequisites-optional.sh "${OS_SHEBANG}\n")

        # Populate the script with the required packages
        if (${MISSING_PACKAGES_SIZE} GREATER "0")
            FOREACH(ITEM IN LISTS MISSING_PACKAGES)
                file(APPEND ${CMAKE_BINARY_DIR}/prerequisites-required.sh "${${ITEM}_install_command}")
                file(APPEND ${CMAKE_BINARY_DIR}/prerequisites-required.sh "\n")
            ENDFOREACH()
        endif()

        # Populate the script with the optional packages (if the user wants so)
        if (${OPTIONAL_PACKAGES_SIZE} GREATER "0")
            FOREACH(ITEM IN LISTS OPTIONAL_PACKAGES)
                file(APPEND ${CMAKE_BINARY_DIR}/prerequisites-optional.sh "${${ITEM}_install_command}")
                file(APPEND ${CMAKE_BINARY_DIR}/prerequisites-optional.sh "\n")
            ENDFOREACH()
        endif()
    endif()

    # Print some information to the final user
    if (${MISSING_PACKAGES_SIZE} GREATER "0")
        PRINT_WARNING()
        MESSAGE("Some prerequisites are missing. Some scripts were created inside ${CMAKE_BINARY_DIR} to solve \n\
this issue. The scripts are:\n
\t * ${CMAKE_BINARY_DIR}/prerequisites-required.sh (for the required packages);\n\
\t * ${CMAKE_BINARY_DIR}/prerequisites-optional.sh (for the optional packages).\n\n\
You can run them and they will install automatically all the requirements for buidling BioDynaMo. You will need\n\
sudo rights to do so. Once the scripts are completed, you will need to run again cmake to detect the newly installed packages.\n\
Remember that you are only required to run prerequisites-required.sh in order to be able to build BioDynaMo.")
        PRINT_LINE()
    else()
        PRINT_LINE()
        MESSAGE("You have just finished to configure BioDynaMo. Now you can run \"make .\" to compile it.\n\
Remember to source ${BDM_INSTALL_DIR}/biodynamo-env.sh before actually running the tests or using\n\
the library.")

        if (${OPTIONAL_PACKAGES_SIZE} GREATER "0")
            MESSAGE("\nWe detected that also some optional packages are missing. In order to install them you just
need to run the following script:\n
\t * ${CMAKE_BINARY_DIR}/prerequisites-optional.sh.\n
It will required sudo rights to work correcly. After running the scripts,
you will need to run again cmake to detect the newly installed packages.")
        ENDIF()

        PRINT_LINE()
    endif()
endfunction()

# This function will create a pre-install directory which will contain all the files
# needed to use BioDynaMo as an external project. Moreover, it will create custom
# targets such to run this procedure after the build process has completed.
function(install_inside_build)

    include(GreatCMakeCookOff/TargetCopyFiles)

    add_custom_target(copy_files_bdm ALL DEPENDS biodynamo)

    # Install the enviroment source script
    if(LINUX)
        configure_file(${CMAKE_SOURCE_DIR}/util/installation/common/biodynamo-linux-env.sh ${CMAKE_BINARY_DIR}/biodynamo-env.sh @ONLY)
    elseif(APPLE)
        configure_file(${CMAKE_SOURCE_DIR}/util/installation/common/biodynamo-macos-env.sh ${CMAKE_BINARY_DIR}/biodynamo-env.sh @ONLY)
    endif()
    add_copy_files(copy_files_bdm
            DESTINATION ${CMAKE_BIODYNAMO_ROOT}
            ${CMAKE_BINARY_DIR}/biodynamo-env.sh
            )

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
            ${CMAKE_SOURCE_DIR}/util/version/version.py
            ${CMAKE_SOURCE_DIR}/util/makefile-build/bdm-config
            ${CMAKE_SOURCE_DIR}/util/makefile-build/bdm-code-generation
            )

    # Copy some cmake files
    add_copy_files(copy_files_bdm
            DESTINATION ${CMAKE_INSTALL_CMAKEDATADIR}
            ${CMAKE_SOURCE_DIR}/cmake/BioDynaMo.cmake
            ${CMAKE_SOURCE_DIR}/cmake/SetCompilerFlags.cmake
            ${CMAKE_SOURCE_DIR}/cmake/FindROOT.cmake
            ${CMAKE_SOURCE_DIR}/cmake/FindVTune.cmake
            ${CMAKE_SOURCE_DIR}/cmake/FindOpenCL.cmake
            ${CMAKE_SOURCE_DIR}/cmake/RootUseFile.cmake
            ${CMAKE_BINARY_DIR}/UseBioDynaMo.cmake
            ${CMAKE_SOURCE_DIR}/cmake/utils.cmake
            )
    add_copy_files(copy_files_bdm
            DESTINATION ${CMAKE_INSTALL_CMAKEDATADIR}
            GLOB ${CMAKE_SOURCE_DIR}/cmake/*.xml
            )
    add_copy_files(copy_files_bdm
            DESTINATION ${CMAKE_INSTALL_CMAKEDIR}
            ${CMAKE_SOURCE_DIR}/cmake/BioDynaMoConfig.cmake
            ${CMAKE_SOURCE_DIR}/cmake/FindNuma.cmake
            ${CMAKE_SOURCE_DIR}/cmake/FindTBB.cmake
            )

    # Other headers
    add_copy_files(copy_files_bdm
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
            ${CMAKE_SOURCE_DIR}/util/version/version.h
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

function(add_bdm_packages_properties)
    SET_PACKAGE_PROPERTIES(MPI PROPERTIES
            DESCRIPTION "OpenMPI, an Open Source Message Passing Interface."
            TYPE REQUIRED
            )
    SET_PACKAGE_PROPERTIES(GLUT PROPERTIES
            DESCRIPTION "Open Source alternative to the OpenGL Utility Toolkit (GLUT) library."
            TYPE REQUIRED
            )
    SET_PACKAGE_PROPERTIES(OpenMP PROPERTIES
            DESCRIPTION "API that enables multi-platform shared memory multiprocessing programming."
            TYPE REQUIRED
            )
    SET_PACKAGE_PROPERTIES(Threads PROPERTIES
            DESCRIPTION "GNU C library POSIX threads implementation."
            TYPE REQUIRED
            )
    SET_PACKAGE_PROPERTIES(Git PROPERTIES
            DESCRIPTION "Open Source Distributed Version Control System."
            TYPE REQUIRED
            )
    SET_PACKAGE_PROPERTIES(ROOT PROPERTIES
            DESCRIPTION "CERN's Modular Scientific Software Toolkit."
            TYPE REQUIRED
            )
    SET_PACKAGE_PROPERTIES(ClangTools PROPERTIES
            DESCRIPTION "Standalone command line tools that provide developer-oriented functionalities."
            TYPE REQUIRED
            )
    SET_PACKAGE_PROPERTIES(Numa PROPERTIES
            DESCRIPTION "Simple API to the NUMA (Non Uniform Memory Access) policy supported by the Linux kernel."
            TYPE REQUIRED
            )
    SET_PACKAGE_PROPERTIES(ParaView PROPERTIES
            DESCRIPTION "Open Source, multi-platform data analysis and visualization application."
            TYPE REQUIRED
            )
    SET_PACKAGE_PROPERTIES(Valgrind PROPERTIES
            DESCRIPTION "A suite of tools for debugging and profiling."
            TYPE REQUIRED
            )
    SET_PACKAGE_PROPERTIES(Doxygen PROPERTIES
            DESCRIPTION "Tool for generating documentation from annotated C++ sources."
            TYPE REQUIRED
            )
    SET_PACKAGE_PROPERTIES(MKDocs PROPERTIES
            DESCRIPTION "Fast, simple and downright gorgeous static site generator that's geared towards building project documentation."
            TYPE REQUIRED
            )
endfunction()

# Helper function to print a simple line
function(print_line)
    MESSAGE("\n################################################################\n")
endfunction()

# Helper function to print a warning message
function(print_warning)
    MESSAGE("\n########################### WARNING ############################\n")
endfunction()