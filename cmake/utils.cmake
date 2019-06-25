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
    else()
        # Check if ROOT was compiled with the correct C++ standard (which has to be greater or equal
        # than C++14). If that's not the case, then we will download the correct version and we will
        # use that instead of the system one.
        if (NOT ROOT_CXX_FLAGS MATCHES ["-std=c++14"|"-std=c++1y"])
            MESSAGE(WARNING "The ROOT version currently installed was compiled with an older c++ standard that is not\
compatible with BioDynaMo. We will proceed to download the correct version of ROOT now.")
            include(external/ROOT)
        endif()

        # Manually set the ROOT enviromental variables if it ROOTSYS not previously found.
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
                SET(BDM_OS "travis-osx" PARENT_SCOPE)
            else()
                SET(BDM_OS "osx" PARENT_SCOPE)
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

function(generate_download_prerequisites OS MISSING_PACKAGES REQUIRED_PACKAGES)

    # We remove both ParaView and ROOT since they are installed automatically
    list(REMOVE_ITEM MISSING_PACKAGES "ParaView" "ROOT")

    # We first get the required packages we are missing
    SET(OPTIONAL_PACKAGES "${MISSING_PACKAGES}")
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

    # Print some information to the final user
    if (${OPTIONAL_PACKAGES_SIZE} GREATER "0" OR ${MISSING_PACKAGES_SIZE} GREATER "0")
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
        PRINT_LINE()
    endif()
endfunction()

function(print_line)
    MESSAGE("\n################################################################\n")
endfunction()

function(print_warning)
    MESSAGE("\n########################### WARNING ############################\n")
endfunction()