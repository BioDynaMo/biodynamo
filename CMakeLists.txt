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

cmake_minimum_required(VERSION 3.5.1 FATAL_ERROR)

set(policy_new CMP0072 CMP0077)
foreach(policy ${policy_new})
  if(POLICY ${policy})
    cmake_policy(SET ${policy} NEW)
  endif()
endforeach()

# Project name and current version. We do not specify any language for
# this project. Therefore, we will be able to check manually which compiler
# is detected on the system.
project(BioDynaMo LANGUAGES VERSION "0.1.0")

#---Set the locale to default C to prevent issued due to localization of commands---------------
# This is necessary as we for example call `clang -v` and parse its output. But on a localized
# program, the output parsing is much more error prone as certrain strings we're looking for
# could be missing or be in a different order. To prevent those errors, let's just force all
# output to use the default C locale which is more or less identical on all systems.
set(ENV{LANG} C)

# Directory in which we saved all the cmake informations.
set(ENV{BDM_CMAKE_DIR} ${CMAKE_SOURCE_DIR}/cmake)

# Add an additional path to make include() look in the right places.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "$ENV{BDM_CMAKE_DIR}")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/third_party/cmake/")

# Include some cmake files with utilities
include(external/SHA256Digests)
include(FeatureSummary)
include(CaptureCommandLine)
include(utils)

# Detect os type and version
if (NOT DEFINED OS)
    SET(DETECTED_OS "none")
    detect_os()
else()
    SET(DETECTED_OS ${OS})
    check_detected_os(${DETECTED_OS})
endif()

# Find a suitable compiler
include(CheckCompiler)

# Directory in which BioDynaMo will be installed. It can be provided
# by the user itself.
if (NOT DEFINED ${BDM_INSTALL_DIR})
    set(BDM_INSTALL_DIR $ENV{HOME})
endif()

# Specify some directories which will be used to structure BioDynaMo build.
set(CMAKE_BIODYNAMO_BUILD_ROOT  "${CMAKE_BINARY_DIR}")
set(CMAKE_INSTALL_ROOT          "${CMAKE_BIODYNAMO_BUILD_ROOT}")
set(CMAKE_THIRD_PARTY_DIR       "${CMAKE_INSTALL_ROOT}/third_party")
set(CMAKE_INSTALL_BINDIR        "${CMAKE_INSTALL_ROOT}/bin")
set(CMAKE_INSTALL_INCLUDEDIR    "${CMAKE_INSTALL_ROOT}/include")
set(CMAKE_INSTALL_LIBDIR        "${CMAKE_INSTALL_ROOT}/lib")
set(CMAKE_INSTALL_PVPLUGINDIR   "${CMAKE_INSTALL_ROOT}/lib/pv_plugin") # Must be in separate dir!
set(CMAKE_INSTALL_CMAKEDIR      "${CMAKE_INSTALL_ROOT}/cmake")
set(CMAKE_INSTALL_DATADIR       "${CMAKE_INSTALL_ROOT}/share")
set(CMAKE_INSTALL_CMAKEDATADIR  "${CMAKE_INSTALL_DATADIR}/cmake")
set(CMAKE_INSTALL_THIRDPARTY    "${CMAKE_INSTALL_ROOT}")

# It is used by format/style checks. It needs to be defined here such
# to enable relocation of the scripts.
set(BUILD_SUPPORT_DIR "${CMAKE_SOURCE_DIR}/util/housekeeping")

#Set paths where to put the libraries, executables and headers
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_INSTALL_ROOT}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_INSTALL_ROOT}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_INSTALL_ROOT}/bin)

# Hide them from configuration tools
mark_as_advanced(${CMAKE_BIODYNAMO_BUILD_ROOT}
        ${CMAKE_INSTALL_ROOT}
        ${CMAKE_INSTALL_BINDIR}
        ${CMAKE_INSTALL_INCLUDEDIR}
        ${CMAKE_INSTALL_LIBDIR}
        ${CMAKE_INSTALL_CMAKEDIR}
        ${CMAKE_INSTALL_DATADIR}
        ${CMAKE_INSTALL_CMAKEDATADIR}
        ${CMAKE_INSTALL_THIRDPARTY})

# Options. Turn on with 'cmake -Dmyvarname=ON'.
option(test      "Build all tests." ON) # Makes boolean 'test' available.
option(cuda      "Enable CUDA code generation for GPU acceleration" OFF)
option(opencl    "Enable OpenCL code generation for GPU acceleration" OFF)
option(dict      "Build with ROOT dictionaries" ON)
option(paraview  "Enable ParaView" ON)
option(notebooks "Generate ROOT notebooks" OFF)
option(sbml      "Enable SBML" OFF)
option(vtune     "Enable VTune performance analysis" OFF)
option(coverage  "Enable test coverage report generation. Sets build type to coverage" OFF)
option(verbose   "Enable verbosity when installing." OFF)
option(tcmalloc  "Use tcmalloc for memory allocations." OFF)
option(website   "Enable website generation (make website<-live>)." OFF)

if(APPLE)
   # valgrind currently broken on macOS 10.12
   option(valgrind "Enable valgrind tests and make build compatible with valgrind tool." OFF)
else()
   option(valgrind "Enable valgrind tests and make build compatible with valgrind tool." ON)
endif()

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
    add_definitions("-DLINUX")
endif()

add_definitions(-DBDM_SRC_DIR=\"${CMAKE_SOURCE_DIR}/src\")  # escaped quotes are mandatory

# -------------------- find packages ------------------------------------------
if (tcmalloc)
  find_package(tcmalloc)
  if(NOT TCMALLOC_FOUND)
    message(FATAL_ERROR "TCMalloc not found.")
  endif()
  message("Tcmalloc libarary found: ${TCMALLOC_LIBRARY_PATH}")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -ltcmalloc -L ${TCMALLOC_LIBRARY_PATH}")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -ltcmalloc -L ${TCMALLOC_LIBRARY_PATH}")
endif()

find_program(WGET_BIN wget)
if (NOT WGET_BIN)
  message(FATAL_ERROR "The program wget was not found in your system. Please run ./prerequisites.sh again before calling cmake.")
endif()
# Check if Python is present
find_package(PythonInterp 3.5)
if (PythonInterp_NOT_FOUND)
    MESSAGE(FATAL_ERROR "We did not find any Python interpreter. Please run ./prerequisites.sh again before
calling cmake. The Python interpreter is required in order to successfully use BioDynaMo." )
endif()
find_package(pip)

# Check if OpenMPI is present
find_package(MPI)
if (MPI_NOT_FOUND)
    MESSAGE(FATAL_ERROR "We did not find any OpenMPI installation. Please run ./prerequisites.sh again before
calling cmake. The OpenMPI library is required in order to successfully use BioDynaMo." )
endif()

# Check if GLUT is present. If we do not have it, then we
# disable directly paraview.
find_package(GLUT)
if(NOT GLUT_FOUND)
    PRINT_WARNING()
    MESSAGE("GLUT libraries were not found in the system. In order to use ParaView they need to be installed.
If you want to do that automatically, please run the ./prerequisites.sh script.")
    PRINT_LINE()
    SET(paraview OFF)
endif()

# Check if we have a compatible openmp compiler
find_package(OpenMP)
if (OpenMP_FOUND)
  get_filename_component(OpenMP_CXX_LIB_DIR "${OpenMP_CXX_LIBRARIES}" DIRECTORY)
  link_directories(${OpenMP_CXX_LIB_DIR})
else()
    MESSAGE(FATAL_ERROR "OpenMP was not found in your system. Please run ./prerequisites.sh again before
calling cmake. If this does not work, then your current compiler might not support OpenMP (this happens for instance
with Apple's standard compiler). Please update it to a more recent version and then run cmake again.")
endif()

if (test)
  # must be before include(${ROOT_USE_FILE}) otherwise test compile fails
  find_package(Threads)
endif()

if(cuda)
  find_package(CUDA)
  if (NOT CUDA_FOUND)
    MESSAGE("CUDA was not found in your system, therefore it was disabled to let you compile BioDynaMo. In order to
enable this functionality please install CUDA for your system. Please have a look at https://docs.nvidia.com/cuda/index.html.
Then run again cmake (with the -Dcuda=ON flag).")
    SET(cuda OFF)
  endif()
endif()

if(opencl)
  find_package(OpenCL)
  if (NOT OpenCL_FOUND)
      MESSAGE("OpenCL was not found in your system, therefore it was disabled to let you compile BioDynaMo. In order to
enable this functionality please install OpenCL for your system. Please have a look at the website of your graphic card vendor on how to install OpenCL.
Then run again cmake (with the -Dopencl=ON flag).")
      SET(opencl OFF)
  endif()
endif()

# Check if we have git installed
find_package(Git)

# Find ROOT and check its installation
find_package(ROOT COMPONENTS Geom Gui Eve GenVector)
if(ROOT_FOUND)
  message("-- ROOT_VERSION = ${ROOT_VERSION}")
endif()
verify_ROOT()

if (dict)
  add_definitions("-DUSE_DICT")
endif()
find_package(ClangTools)
if ("$ENV{CMAKE_EXPORT_COMPILE_COMMANDS}" STREQUAL "1" OR CLANG_TIDY_FOUND)
  # Generate a Clang compile_commands.json "compilation database" file for use
  # with various development tools, such as Vim's YouCompleteMe plugin.
  # See http://clang.llvm.org/docs/JSONCompilationDatabase.html
  set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
endif()

find_package(Numa)
if (NUMA_FOUND)
   include_directories(${NUMA_INCLUDE_DIR})
   link_directories(${NUMA_LIBRARY_DIR})
   add_definitions("-DUSE_NUMA")
   set(BDM_REQUIRED_LIBRARIES ${BDM_REQUIRED_LIBRARIES} ${NUMA_LIBRARIES})
endif()

if(paraview)
    # If we specify a custom location for ParaView then we need to specify also a custom location
    # for Qt.
    if ((DEFINED ENV{ParaView_DIR} AND NOT DEFINED ENV{Qt5_DIR}) OR (NOT DEFINED ENV{ParaView_DIR} AND DEFINED ENV{Qt5_DIR}))
        MESSAGE(FATAL_ERROR "\nFATAL ERROR \nYou need to specify both ParaView_DIR and Qt5_DIR in order to build "
                "BioDynaMo. This is needed only if you want to use your own versions of ParaView and Qt.")
    endif()

    # If Qt5_DIR is not set by the user, we will set it to the default directory
    # where it's downloaded to. This is done in case the user runs CMake again,
    # after which the find_package(ParaView) might complain that it can't find
    # a compatible Qt5 installation
    if(NOT DEFINED Qt5_DIR OR NOT DEFINED ${Qt5_DIR})
      set(Qt5_DIR ${CMAKE_THIRD_PARTY_DIR}/qt/lib/cmake/Qt5)
    endif()

    # Check the first time if ParaView exists in the system.
    find_package(ParaView 5.3 OPTIONAL_COMPONENTS vtkPVPythonCatalyst
            vtkPVVTKExtensionsDefault vtkPVServerManagerRendering
            PATHS ${CMAKE_THIRD_PARTY_DIR}/paraview)

    # Check if ParaView was found in the system. If that's not the case then we will proceed
    # to download it.
    if(NOT ${ParaView_FOUND})
      PRINT_WARNING()
      MESSAGE("ParaView was not found in the system. Therefore it will be installed automatically now "
              "together with Qt5.")
      PRINT_LINE()

      # If we did not found paraview then we will install it automatically
      # with the Qt package.
      include(external/Qt)
      PRINT_LINE()
      MESSAGE("Qt5 was successfully downloaded and installed. We will proceed now with ParaView.")
      PRINT_LINE()
      include(external/ParaView)

      # We run again the find_package command such to find correctly the location of Qt and ParaView
      find_package(Qt5 COMPONENTS Widgets HINTS ${CMAKE_THIRD_PARTY_DIR}/qt/lib/cmake/Qt5 NO_CMAKE_PATH NO_CMAKE_ENVIRONMENT_PATH)
      if(Qt5_FOUND)
        message("-- Found Qt5 (version ${Qt5_VERSION})")
      endif()
      find_package(ParaView 5.3 OPTIONAL_COMPONENTS vtkPVPythonCatalyst
              vtkPVVTKExtensionsDefault vtkPVServerManagerRendering
              PATHS ${CMAKE_THIRD_PARTY_DIR}/paraview)
    endif()

    # Add several ParaView related options
    include("${PARAVIEW_USE_FILE}")
    add_definitions("-DUSE_PARAVIEW")
    # link_directories($ENV{ParaView_LIB_DIR})
    # set(BDM_REQUIRED_LIBRARIES ${BDM_REQUIRED_LIBRARIES} vtkPVPythonCatalyst
    #       vtkIOParallelXML vtkPVVTKExtensionsDefault vtkPVServerManagerRendering)
endif()

# paraview version 5.5 (MacOS) does not contain TBB
find_package(TBB)
if (TBB_FOUND)
  include_directories(${TBB_INCLUDE_DIRS})
  set(BDM_REQUIRED_LIBRARIES ${BDM_REQUIRED_LIBRARIES} ${TBB_LIBRARIES})
else()
  MESSAGE(FATAL_ERROR "TBB was not found on your system."
  " Please run ./prerequisites.sh again before calling cmake.")
endif()

if(sbml)
  if(APPLE)
    message(FATAL_ERROR "Currently SBML is not supported on MacOS (see https://trello.com/c/vKPbh4iG).")
  endif()
  find_package(Libroadrunner)
  if(NOT Libroadrunner_FOUND)
    include(external/Libroadrunner)
  endif()
endif()

if (vtune)
    find_package(VTune)
    if(${VTune_FOUND})
        include_directories(${VTUNE_INCLUDE_DIR})
        add_definitions("-DUSE_VTUNE")
        set(BDM_REQUIRED_LIBRARIES ${BDM_REQUIRED_LIBRARIES} ${VTUNE_LIBRARIES})
    else()
        MESSAGE("VTune was not found in your system, therefore it was disabled. You will not be able to run the perfomance
checks. In order to enable this functionality please install VTune for your system. Please have a look at https://software.intel.com/en-us/vtune.
Then run again cmake (with the -Dvtune=ON flag).")
    endif()
endif()

# Check if we can find the valgrind executable
if (valgrind)
    find_package(Valgrind QUIET)
    if (NOT VALGRIND_FOUND)
        MESSAGE("valgrind was not found in your system, therefore it was disabled. You will not be able to run the memory
checks. In order to enable this functionality please install valgrind for your system. This can be done automatically with
the prerequistes.sh script offered by the library. Then run again cmake (with the -Dvalgrind=ON flag).")
        SET(valgrind OFF)
    endif()
endif()

# Link to OpenCL
if (OPENCL_FOUND)
  if (OPENCL_HAS_CXX OR APPLE)
    add_definitions("-DUSE_OPENCL")
    set(BDM_REQUIRED_LIBRARIES ${BDM_REQUIRED_LIBRARIES} ${OPENCL_LIBRARIES})
  else()
    message(WARNING "OpenCL C++ bindings not found. Please install to make use of OpenCL. "
      "If you think you have installed the C++ bindings correctly, please check if one "
      "of the following environmentals is set correctly (vendor specific):
      - AMD: \t\tAMDAPPSDKROOT
      - NVIDIA: \tCUDA_PATH
      - INTEL: \tINTELOPENCLSDK")
    set(OPENCL_FOUND FALSE)
  endif()
endif()

# --------------------Copy the launcher script ---------------------------------
configure_file("cmake/env/launcher.sh" "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/launcher.sh" @ONLY)
file(COPY ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/launcher.sh
        DESTINATION ${CMAKE_BINARY_DIR}
        FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ
        GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

# -------------------- includes -----------------------------------------------
include(BioDynaMo)
include(ExternalProject)
include(Documentation)
include(Website)
include(Versioning)

# -------------------- set default build type and compiler flags ---------------
if(coverage)
  set(CMAKE_BUILD_TYPE Coverage)
elseif(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()
include(SetCompilerFlags)

# -------------------- set rpath options ---------------------------------------
# When building, use the RPATH
set(CMAKE_INSTALL_RPATH "")
set(CMAKE_BUILD_RPATH "")
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_SKIP_BUILD_RPATH TRUE) # skip the full RPATH for the build tree
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
set(CMAKE_MACOSX_RPATH TRUE) # use @rpath keyword for the MacOS rpath
# Sets @rpath for libbiodynamo.dylib on Mac OS in the build directory
if (APPLE)
  SET(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
endif()

# -------------------- extract third party libraries ---------------------------
set(EXTRACTED_THIRD_PARTY_LIBS "${CMAKE_CURRENT_BINARY_DIR}/extracted-third-party-libs")
file(MAKE_DIRECTORY "${EXTRACTED_THIRD_PARTY_LIBS}/")
execute_process(COMMAND ${CMAKE_COMMAND} -E tar xf "${CMAKE_SOURCE_DIR}/third_party/mpark_4ee0086.zip"
                WORKING_DIRECTORY "${EXTRACTED_THIRD_PARTY_LIBS}")
execute_process(COMMAND ${CMAKE_COMMAND} -E tar xf "${CMAKE_SOURCE_DIR}/third_party/libmorton-7ff3866.zip"
                WORKING_DIRECTORY "${EXTRACTED_THIRD_PARTY_LIBS}")

# -------------------- build shared library libbiodynamo -----------------------
# add the headers into the project
include(${ROOT_USE_FILE})
include_directories("${CMAKE_SOURCE_DIR}/src")
include_directories("${CMAKE_SOURCE_DIR}/test")
include_directories("${CMAKE_SOURCE_DIR}/third_party")
include_directories("${CMAKE_SOURCE_DIR}/third_party/omp")
include_directories("${CMAKE_SOURCE_DIR}/third_party/opencl")
include_directories("${CMAKE_SOURCE_DIR}/third_party/cxxopts-v2.2.0")
include_directories("${EXTRACTED_THIRD_PARTY_LIBS}")
include_directories("${CMAKE_CURRENT_BINARY_DIR}/version")

# Since including the directory where omp.h resides can cause rootcling to
# search for system / C++ standard headers in that same directory, we instead
# copy omp.h to our local build directory and include that
fix_rootcling_omp_issue()

# build libbiodynamo
file(GLOB_RECURSE HEADERS "${CMAKE_SOURCE_DIR}/src/*.h")
file(GLOB_RECURSE LIB_SOURCES "${CMAKE_SOURCE_DIR}/src/*.cc")
file(GLOB_RECURSE KERNELS "${CMAKE_SOURCE_DIR}/src/*.cu")

# Exclude paraview-related source files, which will be loaded dynamically as a
# plugin upon request
list(FILTER LIB_SOURCES EXCLUDE REGEX "paraview/*")
list(FILTER HEADERS EXCLUDE REGEX "paraview/*")
file(GLOB_RECURSE PV_HEADERS "${CMAKE_SOURCE_DIR}/src/core/visualization/paraview/*.h")
file(GLOB_RECURSE PV_SOURCES "${CMAKE_SOURCE_DIR}/src/core/visualization/paraview/*.cc")

build_plugin(ParaviewAdaptor
                   SOURCES ${PV_SOURCES}
                   HEADERS ${PV_HEADERS}
                   LIBRARIES vtkCommonCore vtkPVPythonCatalyst vtkIOParallelXML vtkPVVTKExtensionsDefault vtkPVServerManagerRendering)
# add_library(ParaviewAdaptor SHARED ${PV_SOURCES})
# if(${ParaView_FOUND})
#   link_directories($ENV{ParaView_LIB_DIR})
#   target_link_libraries(ParaviewAdaptor vtkPVPythonCatalyst
#         vtkIOParallelXML vtkPVVTKExtensionsDefault vtkPVServerManagerRendering)
# endif()

build_libbiodynamo(biodynamo
                   SOURCES ${LIB_SOURCES}
                   HEADERS ${HEADERS}
                   LIBRARIES ${BDM_REQUIRED_LIBRARIES} ${ROOT_LIBRARIES})
if(${VTune_FOUND})
  target_link_libraries(biodynamo ${VTUNE_LIBRARIES})
endif()

set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS 1 CACHE BOOL "Suppress cmake development warnings")

if (CUDA_FOUND)
  # Check if the necessary CUDA headers files are installed on this machine
  set(EXTRA_CUDA_HEADERS "${CUDA_TOOLKIT_ROOT_DIR}/samples/common/inc/helper_math.h")
  if(NOT EXISTS ${EXTRA_CUDA_HEADERS})
    message(FATAL_ERROR "Could not find ${EXTRA_CUDA_HEADERS} "
                        "Please make sure you installed CUDA correctly")
  else()
    add_definitions("-DUSE_CUDA")

    # Detect CUDA architecture and get best NVCC flags
    if(NOT COMMAND CUDA_SELECT_NVCC_ARCH_FLAGS)
      INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/cmake/select_compute_arch.cmake)
    endif()
    CUDA_SELECT_NVCC_ARCH_FLAGS(NVCC_FLAGS_EXTRA $ENV{TORCH_CUDA_ARCH_LIST})
    list(APPEND CUDA_NVCC_FLAGS ${NVCC_FLAGS_EXTRA})

    set(CUDA_HOST_COMPILER "/usr/bin/")
    set(NVCCFLAGS "--compiler-bindir ${CUDA_HOST_COMPILER}")
    if(CMAKE_BUILD_TYPE MATCHES DEBUG)
      set(CUDA_NVCC_FLAGS "${CUDA_NVCC_FLAGS} -g -G -O1" )
    endif()
    set(CUDA_NVCC_FLAGS "${CUDA_NVCC_FLAGS} -std=c++11")

    target_link_libraries(biodynamo ${CUDA_LIBRARIES})
    cuda_include_directories(${CUDA_INCLUDE_DIRS} ${CUDA_TOOLKIT_ROOT_DIR})
    include_directories(${CUDA_INCLUDE_DIRS} ${CUDA_TOOLKIT_ROOT_DIR})
    cuda_add_library(bdmcuda ${KERNELS})
    target_link_libraries(biodynamo bdmcuda)
  endif()
endif()

# add custom clean target for shared library
add_custom_target(libbdmclean COMMAND ${CMAKE_COMMAND} -P "${CMAKE_BINARY_DIR}/CMakeFiles/biodynamo.dir/cmake_clean.cmake")
# custom clean target for biodynamo that excludes ExternalProjects
add_custom_target(bdmclean)
add_dependencies(bdmclean libbdmclean)

generate_rootlogon()
if (${ParaView_FOUND})
    include("${CMAKE_SOURCE_DIR}/paraview_plugin/bdm_glyph/CMakeLists.txt")
endif()

# create cmake files used by external builds
configure_file("cmake/UseBioDynaMo.cmake.in" "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/UseBioDynaMo.cmake" @ONLY)

# -------------------- build test project  ------------------------------------
if (test)
  enable_testing()

  include(Testing)

  # unit tests
  #   main test executable
  file(GLOB TEST_SOURCES ${CMAKE_SOURCE_DIR}/test/unit/*.cc
                         ${CMAKE_SOURCE_DIR}/test/unit/test_util/*.cc
                         ${CMAKE_SOURCE_DIR}/test/unit/core/*.cc
                         ${CMAKE_SOURCE_DIR}/test/unit/core/biology_module/*.cc
                         ${CMAKE_SOURCE_DIR}/test/unit/core/container/*.cc
                         ${CMAKE_SOURCE_DIR}/test/unit/core/execution_context/*.cc
                         ${CMAKE_SOURCE_DIR}/test/unit/core/operation/*.cc
                         ${CMAKE_SOURCE_DIR}/test/unit/core/param/*.cc
                         ${CMAKE_SOURCE_DIR}/test/unit/core/sim_object/*.cc
                         ${CMAKE_SOURCE_DIR}/test/unit/core/util/*.cc
                         ${CMAKE_SOURCE_DIR}/test/unit/core/visualization/*.cc
                         ${CMAKE_SOURCE_DIR}/test/unit/neuroscience/*.cc)
  file(GLOB TEST_HEADERS ${CMAKE_SOURCE_DIR}/test/unit/*.h
                         ${CMAKE_SOURCE_DIR}/test/unit/test_util/*.h
                         ${CMAKE_SOURCE_DIR}/test/unit/core/*.h
                         ${CMAKE_SOURCE_DIR}/test/unit/core/biology_module/*.h
                         ${CMAKE_SOURCE_DIR}/test/unit/core/container/*.h
                         ${CMAKE_SOURCE_DIR}/test/unit/core/execution_context/*.h
                         ${CMAKE_SOURCE_DIR}/test/unit/core/operation/*.h
                         ${CMAKE_SOURCE_DIR}/test/unit/core/param/*.h
                         ${CMAKE_SOURCE_DIR}/test/unit/core/operation/*.h
                         ${CMAKE_SOURCE_DIR}/test/unit/core/sim_object/*.h
                         ${CMAKE_SOURCE_DIR}/test/unit/core/util/*.h
                         ${CMAKE_SOURCE_DIR}/test/unit/core/visualization/*.h
                         ${CMAKE_SOURCE_DIR}/test/unit/neuroscience/*.h)
  bdm_add_test_executable(biodynamo-unit-tests
                          SOURCES ${TEST_SOURCES}
                          HEADERS ${TEST_HEADERS})
  #   separate test binaries - each source file is compiled into a separate binary
  # file(GLOB TEST_SOURCES ${CMAKE_SOURCE_DIR}/test/unit/separate_binary/*.cc)
  # file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/empty_header.h "")
  # foreach(file ${TEST_SOURCES})
  #   get_filename_component(filename ${file} NAME_WE)
  #   set(TEST_HEADER ${CMAKE_SOURCE_DIR}/test/unit/separate_binary/${filename}.h)
  #   if (NOT EXISTS ${TEST_HEADER})
  #     set(TEST_HEADER ${CMAKE_CURRENT_BINARY_DIR}/empty_header.h)
  #   endif()
  #   bdm_add_test_executable("runBiodynamoTests_${filename}"
  #                           SOURCES ${file}
  #                           HEADERS ${TEST_HEADER})
  # endforeach()

  # integration tests
  if (NOT coverage)
    if(OPENCL_FOUND OR CUDA_FOUND)
      bdm_add_executable(cell_division_gpu
                         SOURCES test/system/cell_division_gpu/src/cell_division_gpu.cc
                         HEADERS test/system/cell_division_gpu/src/cell_division_gpu.h
                         LIBRARIES biodynamo)
      add_dependencies(check cell_division_gpu)
      add_test(NAME "system-cell-division-gpu"
               COMMAND ${CMAKE_SOURCE_DIR}/build/cell_division_gpu)
    endif()
  endif()
endif()

# -------------------- Generate notebooks --------------------------------------
include(Notebooks)

# -------------------- Install configuration -----------------------------------
include(Installation)

# -------------------- add targets for code style and submission checks --------
include(CppStyleGuideChecks)

# ------ cleanbuild command ------ #
add_custom_target(cleanbuild
                  COMMAND ${PROJECT_SOURCE_DIR}/util/clean-build-dir.sh "${CMAKE_BINARY_DIR}"
)

# --------------------- Final Message to the user ------------------------------
add_bdm_packages_properties()
add_bdm_feature_properties()
print_summary()
feature_summary(INCLUDE_QUIET_PACKAGES
        DESCRIPTION "The following packages were found:"
        WHAT PACKAGES_FOUND)
feature_summary(INCLUDE_QUIET_PACKAGES
        DESCRIPTION "The following packages were not found:"
        WHAT PACKAGES_NOT_FOUND)
feature_summary(WHAT ENABLED_FEATURES
        INCLUDE_QUIET_PACKAGES
        DESCRIPTION "Enabled Features:")
feature_summary(WHAT DISABLED_FEATURES
        INCLUDE_QUIET_PACKAGES
        DESCRIPTION "Disabled Features:")

# Install all the files inside the build dir
install_inside_build()

# Copy the environmental files to their correct directory
configure_file(${CMAKE_SOURCE_DIR}/cmake/env/thisbdm.sh ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/thisbdm.sh @ONLY NEWLINE_STYLE UNIX)

# Set run permission on the environmental files
add_permissions("${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/thisbdm.sh" ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

PRINT_LINE()
MESSAGE("You have just finished to configure BioDynaMo. Now you can run \"make .\" to compile it.\n\
You can then call \"make install\" to install in your system.
Remember to source ${BDM_INSTALL_DIR}/bin/thisbdm.sh before using the library.")
PRINT_LINE()
