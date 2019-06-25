include(ExternalProject)

# Directory in which root will be donwloaded first (the path
# should be something like <build_dir>/third_party/...).
SET(ROOT_SOURCE_DIR "${CMAKE_BINARY_DIR}/third_party/root")

ExternalProject_Add(
        ROOT
        PREFIX ${CMAKE_BINARY_DIR}/third_party
        DOWNLOAD_DIR ${CMAKE_BINARY_DIR}/third_party
        SOURCE_DIR ${ROOT_SOURCE_DIR}/
        URL http://cern.ch/biodynamo-lfs/third-party/${DETECTED_OS}/root.tar.gz
        URL_MD5
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory ${ROOT_SOURCE_DIR} ${THIRD_PARTY_DIR}/root
)

# Set the include dir
SET(ROOT_INCLUDE_DIR ${THIRD_PARTY_DIR}/root/include PARENT_SCOPE)
SET(ROOT_INCLUDE_DIRS ${THIRD_PARTY_DIR}/root/include PARENT_SCOPE)

# Build a set with all the ROOT libraries needed
set(rootlibs Core RIO Net Hist Graf Graf3d Gpad Tree Rint Postscript Matrix Physics MathCore Thread MultiProc)
set(ROOT_LIBRARIES PARENT_SCOPE)
foreach(_cpt ${rootlibs} ${ROOT_FIND_COMPONENTS})
    find_library(ROOT_${_cpt}_LIBRARY ${_cpt} HINTS ${ROOT_LIBRARY_DIR})
    if(ROOT_${_cpt}_LIBRARY)
        mark_as_advanced(ROOT_${_cpt}_LIBRARY)
        list(APPEND ROOT_LIBRARIES ${ROOT_${_cpt}_LIBRARY})
        if(ROOT_FIND_COMPONENTS)
            list(REMOVE_ITEM ROOT_FIND_COMPONENTS ${_cpt})
        endif()
    endif()
endforeach()
if(ROOT_LIBRARIES)
    list(REMOVE_DUPLICATES ROOT_LIBRARIES)
endif()

# Set ROOTSYS variable
SET(ENV{ROOTSYS} "${THIRD_PARTY_DIR}/root" PARENT_SCOPE)

# Set LD_LIBRARY_PATH variable
set(ENV{LD_LIBRARY_PATH} "$ENV{ROOTSYS}/lib:$ENV{LD_LIBRARY_PATH}" PARENT_SCOPE)

# Since the user did not set the $ROOTSYS variable, then we need to source
# the ROOT environment every time we need it.
source_root_file(${PROJECT_BINARY_DIR})
set(CUSTOM_ROOT_SOURCE_ENV TRUE PARENT_SCOPE)
set(CUSTOM_ROOT_SOURCE_ENV_COMMAND . ${PROJECT_BINARY_DIR}/source_root_auto.sh PARENT_SCOPE)

