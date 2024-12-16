# -----------------------------------------------------------------------------
#
# Copyright (C) 2021 CERN & University of Surrey for the benefit of the
# BioDynaMo collaboration. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
#
# See the LICENSE file distributed with this work for details.
# See the NOTICE file distributed with this work for additional information
# regarding copyright ownership.
#
# -----------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.19.3 FATAL_ERROR)

set(policy_new CMP0072 CMP0077)
foreach(policy ${policy_new})
  if(POLICY ${policy})
    cmake_policy(SET ${policy} NEW)
  endif()
endforeach()

if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.24)
  cmake_policy(SET CMP0135 OLD)
endif()

# Project name and current version. We do not specify any language for
# this project. Therefore, we will be able to check manually which compiler
# is detected on the system.
project(BioDynaMo LANGUAGES VERSION "1.05.0")

#---Set the locale to default C to prevent issued due to localization of commands---------------
# This is necessary as we for example call `clang -v` and parse its output. But on a localized
# program, the output parsing is much more error prone as certrain strings we're looking for
# could be missing or be in a different order. To prevent those errors, let's just force all
# output to use the default C locale which is more or less identical on all systems.
set(ENV{LANG} C)

# Directory in which we saved all the cmake informations.
set(BDM_CMAKE_DIR ${CMAKE_SOURCE_DIR}/cmake)

# Add an additional path to make include() look in the right places.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${BDM_CMAKE_DIR}")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/third_party/cmake/")

# Include some cmake files with utilities
include(external/SHA256Digests)
include(FeatureSummary)
include(CaptureCommandLine)
include(utils)

# Detect os type and version
if (NOT DEFINED OS)
    SET(DETECTED_OS "none")
    SET(DETECTED_ARCH "none")
    SET(DETECTED_OS_VERS "none")
    detect_os()
else()
    SET(DETECTED_OS ${OS})
    SET(DETECTED_ARCH "x86_64")
    SET(DETECTED_OS_VERS ${OS})
    check_detected_os(${DETECTED_OS})
endif()
message(STATUS "Detected OS: ${DETECTED_OS_VERS}")

if(APPLE)
  find_program(BREW_BIN brew)
  if(BREW_BIN)
    execute_process(COMMAND ${BREW_BIN} --prefix
                    OUTPUT_VARIABLE BREW_PREFIX
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND ${BREW_BIN} --prefix llvm
                    OUTPUT_VARIABLE LLVM_BREW_PREFIX
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND ${BREW_BIN} --prefix libomp
                    OUTPUT_VARIABLE LIBOMP_BREW_PREFIX
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
  endif()
  # Necessary to find the OpenMP library
  set(ENV{LDFLAGS} "$ENV{LDFLAGS} -L${LIBOMP_BREW_PREFIX}/lib")
endif(APPLE)

# Find a suitable compiler
include(CheckCompiler)

# Specify some directories which will be used to structure BioDynaMo build.
set(CMAKE_BIODYNAMO_BUILD_ROOT  "${CMAKE_BINARY_DIR}")
set(CMAKE_INSTALL_ROOT          "${CMAKE_BIODYNAMO_BUILD_ROOT}")
set(CMAKE_THIRD_PARTY_DIR       "${CMAKE_INSTALL_ROOT}/third_party")
set(CMAKE_INSTALL_BINDIR        "${CMAKE_INSTALL_ROOT}/bin")
set(CMAKE_INSTALL_INCLUDEDIR    "${CMAKE_INSTALL_ROOT}/include")
set(CMAKE_INSTALL_LIBDIR        "${CMAKE_INSTALL_ROOT}/lib")
set(CMAKE_INSTALL_PVPLUGINDIR   "${CMAKE_INSTALL_ROOT}/lib/pv_plugin") # Must be in separate dir!
set(CMAKE_BDM_PVPLUGINDIR       "${CMAKE_SOURCE_DIR}/paraview_plugin/")
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
option(benchmark "Build benchmark suite." OFF)
option(cuda      "Enable CUDA code generation for GPU acceleration" OFF)
option(opencl    "Enable OpenCL code generation for GPU acceleration" OFF)
option(dict      "Build with ROOT dictionaries" ON)
option(paraview  "Enable ParaView" ON)
option(notebooks "Generate BioDynaMo notebooks" OFF)
option(numa      "Enable NUMA-awareness in BioDynaMo." ON)
option(sbml      "Enable SBML" OFF)
option(libgit2   "Enable automatic git documentation for simulations." OFF)
option(vtune     "Enable VTune performance analysis" OFF)
option(coverage  "Enable test coverage report generation. Sets build type to coverage" OFF)
option(verbose   "Enable verbosity when installing." OFF)
option(tcmalloc  "Use tcmalloc for memory allocations." OFF)
option(jemalloc  "Use jemalloc for memory allocations." OFF)
option(website   "Enable website generation (make website<-live>)." OFF)
option(valgrind  "Enable valgrind tests and make build compatible with valgrind tool." ON)
option(rpath     "Link libraries with built-in RPATH (run-time search path)." OFF)
option(real_t    "Define data type for real numbers. Currently supported: float, double" double)

if(APPLE)
  # ParaView on Apple devices
  set(CMAKE_BDM_PVVERSION "5.10")
  
  # Ignore Fink (/sw) and MacPorts (/opt/local) packages, only consider Brew
  set(CMAKE_IGNORE_PATH)
  foreach(_prefix /sw /opt/local)
    list(APPEND CMAKE_IGNORE_PATH ${_prefix}/bin ${_prefix}/include ${_prefix}/lib)
  endforeach()
endif()

if(UNIX AND NOT APPLE)
  set(CMAKE_BDM_PVVERSION "5.9")
  set(LINUX TRUE)
  add_definitions("-DLINUX")
endif()

set(CMAKE_VISIBILITY_INLINES_HIDDEN "ON")

if(real_t)
  message(STATUS "Setting real_t to: ${real_t}")
  add_definitions("-DBDM_REALT=${real_t}")
  add_definitions("-DBDM_CL_REALT=cl_${real_t}")
  set(BDM_CONFIG_REALT "${real_t}")
  if(notebooks AND real_t STREQUAL "float")
    # This if statement turns of the notebooks when we run with real_t=float.
    # Currently, the notebooks do not support BDM with float for unknown reasons.
    message(WARNING "Notebooks are not supported with real_t=float. Disabling notebooks.")
    set(notebooks OFF)
  endif()
else()
  message(STATUS "Using default real_t (double)")
  set(BDM_CONFIG_REALT "float")
endif()

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
if (jemalloc)
  find_package(jemalloc)
  if(NOT JEMALLOC_FOUND)
    message(FATAL_ERROR "jemalloc not found.")
  endif()
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -ljemalloc -L ${JEMALLOC_LIBRARY_DIR}")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -ljemalloc -L ${JEMALLOC_LIBRARY_DIR}")
endif()

if (APPLE)
   set(NOPYENV YES)
endif()


if(NOPYENV)
   # On macOS
   # need to give hint which binary to find, however ideally it should be taken from the range spec 
   set(Python3_EXECUTABLE "${BREW_PREFIX}/bin/python3.9")
   find_package(Python3 3.9.1...<3.10 COMPONENTS Interpreter Development)
   set(pythonvers ${Python_VERSION})
   if (NOT Python3_FOUND)
      message(FATAL_ERROR "We did not find the Python 3.9 interpreter. "
         "Please run ./prerequisites.sh (or \"brew install python@3.9\") "
         "again before calling cmake. The Python 3.9 interpreter "
         "is required in order to successfully use BioDynaMo.")      
   endif()
else()
   # Check if Python 3.9.1 from PyEnv is installed
   set(pythonvers "3.9.1")
   find_package(Python ${pythonvers} EXACT COMPONENTS Interpreter Development)
   if (NOT Python_FOUND)
      message(FATAL_ERROR "We did not find the Python ${pythonvers} interpreter from PyEnv. "
         "Make sure you executed \"pyenv shell ${pythonvers}\" before running cmake. "
         "If that doesn't work, run ./prerequisites.sh again. The Python ${pythonvers} "
         "interpreter is required in order to successfully use BioDynaMo."
         "You may also try the following sequence of commands that has often resolved"
         "the problem: \n"
         "export PATH=\"$HOME/.pyenv/bin:$PATH\" \n"
         "eval \"$(pyenv init --path)\" \n"
         "eval \"$(pyenv init -)\" \n"
         "pyenv shell ${pythonvers}")
   endif()
endif()

# Check if OpenMPI is present
find_package(MPI COMPONENTS C CXX)
if(MPI_FOUND)
  set(CMAKE_CXX_COMPILER ${MPI_CXX_COMPILER})
  include_directories(${MPI_CXX_COMPILER_INCLUDE_DIRS})
  add_definitions("-DUSE_MPI")
else(MPI_FOUND)
  MESSAGE(FATAL_ERROR "We did not find any OpenMPI installation. Please run ./prerequisites.sh again before
  calling cmake. The OpenMPI library is required in order to successfully use BioDynaMo." )
endif(MPI_FOUND)

if(APPLE)
  set(numa OFF)
endif()

# Check if GLUT is present. If we do not have it, then we disable directly paraview.
if(paraview)
  find_package(GLUT)
  if(NOT GLUT_FOUND)
      PRINT_WARNING()
      MESSAGE("GLUT libraries were not found. In order to use ParaView they need to be installed.
 Please run ./prerequisites.sh before calling cmake.")
      PRINT_LINE()
      SET(paraview OFF)
  endif()
endif()

if(APPLE)
  # After migrating to libomp 15.0.3, libomp is no longer symlinked into 
  # $(brew --prefix)/lib/libomp.dylib. Instead, it is located in 
  # $(brew --prefix libomp)/lib . This fix should work accross all platforms.
  # find_package(OpenMP) can be guided with OpenMP_<lang>_INCLUDE_DIR.
  set(CMAKE_CPP_FLAGS "${CMAKE_CPP_FLAGS} -I${LIBOMP_BREW_PREFIX}/include")
  set(OpenMP_C_INCLUDE_DIR "${LIBOMP_BREW_PREFIX}/include")
  set(OpenMP_CXX_INCLUDE_DIR "${LIBOMP_BREW_PREFIX}/include")
endif(APPLE)

# Check if we have a compatible openmp compiler
find_package(OpenMP)
if (OpenMP_CXX_FOUND)
  get_filename_component(OpenMP_CXX_LIB_DIR "${OpenMP_CXX_LIBRARIES}" DIRECTORY)
  link_directories(${OpenMP_CXX_LIB_DIR})
  set(BDM_REQUIRED_LIBRARIES ${BDM_REQUIRED_LIBRARIES} OpenMP::OpenMP_CXX)
else()
  MESSAGE(FATAL_ERROR "OpenMP support was not found. Please run ./prerequisites.sh before
calling cmake.")
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
find_package(ROOT COMPONENTS Geom Gui GenVector)
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

if(numa)
  find_package(Numa)
  if (NUMA_FOUND)
    include_directories(${NUMA_INCLUDE_DIR})
    link_directories(${NUMA_LIBRARY_DIR})
    add_definitions("-DUSE_NUMA")
    set(BDM_REQUIRED_LIBRARIES ${BDM_REQUIRED_LIBRARIES} ${NUMA_LIBRARY})
  endif()
endif()

if(paraview)
    # If we specify a custom location for ParaView then we need to specify also a custom location for Qt.
    if ((DEFINED ENV{ParaView_DIR} AND NOT DEFINED ENV{Qt5_DIR}) OR (NOT DEFINED ENV{ParaView_DIR} AND DEFINED ENV{Qt5_DIR}))
        MESSAGE(FATAL_ERROR "\nFATAL ERROR \nYou need to specify both ParaView_DIR and Qt5_DIR in order to build "
                "BioDynaMo. This is needed only if you want to use your own versions of ParaView and Qt.")
    endif()

    # If Qt5_DIR is not set by the user, we will set it to the default directory
    # where it's downloaded to. This is done in case the user runs CMake again,
    # after which the find_package(ParaView) might complain that it can't find
    # a compatible Qt5 installation
    if(NOT DEFINED Qt5_DIR OR NOT DEFINED ${Qt5_DIR})
      if(APPLE)
         set(Qt5_DIR ${BREW_PREFIX}/opt/qt@5/lib/cmake/Qt5) 
         set(QT_CMAKE_DIR ${BREW_PREFIX}/opt/qt@5/lib/cmake/Qt5)
         set(DYLD_LIBRARY_PATH ${DYLD_LIBRARY_PATH} ${BREW_PREFIX}/opt/qt@5/lib)
      else()
        set(Qt5_DIR ${CMAKE_THIRD_PARTY_DIR}/qt/lib/cmake/Qt5)
      endif(APPLE)
    endif()
    # Check the first time if ParaView exists in the system.
    find_package(ParaView ${CMAKE_BDM_PVVERSION} CONFIG COMPONENTS Catalyst PythonCatalyst VTKExtensionsIOCore
                 PATHS ${CMAKE_THIRD_PARTY_DIR}/paraview ENV{ParaView_DIR})

    if(ParaView_FOUND)
        # Check if found ParaView is BDM installed (matchres > -1)
        string(FIND ${ParaView_PREFIX_PATH} ${CMAKE_THIRD_PARTY_DIR} matchres)
        if (${matchres} GREATER -1)
            # check SHA256 of ParaView to see if it matches currently supported ParaView
            if (IS_DIRECTORY ${CMAKE_THIRD_PARTY_DIR}/paraview)
                if (EXISTS ${CMAKE_THIRD_PARTY_DIR}/paraview/tar-sha256)
                    # check if SHA256 of installed ParaView is the same as the expected one
                    file(READ ${CMAKE_THIRD_PARTY_DIR}/paraview/tar-sha256 TAR_SHA256)
                    if(NOT "${TAR_SHA256}" STREQUAL "${${DETECTED_OS_VERS}-ParaView}")
                        # BDM installed ParaView has wrong SHA256... deleting it
                        message(WARNING "The found ParaView version is not compatible... deleting it...")
                        file(REMOVE_RECURSE ${CMAKE_THIRD_PARTY_DIR}/paraview)
                        file(REMOVE_RECURSE ${CMAKE_THIRD_PARTY_DIR}/qt)
                        unset(ParaView_FOUND)
                    endif()
                else()
                    # BDM installed ParaView exists but no SHA256 file... deleting it
                    message(WARNING "The found ParaView version cannot be determined... deleting it...")
                    file(REMOVE_RECURSE ${CMAKE_THIRD_PARTY_DIR}/paraview)
                    file(REMOVE_RECURSE ${CMAKE_THIRD_PARTY_DIR}/qt)
                    unset(ParaView_FOUND)
                endif()
            endif()
        endif()
    endif()

    if(NOT ParaView_FOUND)

      # If we did not found paraview then we will install it automatically with the Qt package.
      if(APPLE)
        PRINT_WARNING()
        MESSAGE("We did not find any ParaView installed on the system. We will proceed to download it. "
                "ParaView will be installed in the location ${CMAKE_THIRD_PARTY_DIR}/paraview. "
                "Using brew Qt5 in ${Qt5_DIR} for ParaView.")
        PRINT_LINE()
      else()
        PRINT_WARNING()
        MESSAGE("We did not find any ParaView installed on the system. We will proceed to download it "
                "together with Qt5. ParaView will be installed in the location ${CMAKE_THIRD_PARTY_DIR}/paraview.")
        PRINT_LINE()
        include(external/Qt)
        PRINT_LINE()
        MESSAGE("Qt5 was successfully downloaded and installed. We will proceed now with ParaView.")
      endif(APPLE)
      PRINT_LINE()
      include(external/ParaView)

      # We run again the find_package command such to find correctly the location of Qt and ParaView
      find_package(Qt5 REQUIRED COMPONENTS Widgets HINTS ${Qt5_DIR} NO_CMAKE_PATH NO_CMAKE_ENVIRONMENT_PATH)
      if(Qt5_FOUND)
        message("-- Found Qt5 (version ${Qt5_VERSION})")
      endif()
      find_package(ParaView ${CMAKE_BDM_PVVERSION} CONFIG COMPONENTS Catalyst PythonCatalyst VTKExtensionsIOCore
                   PATHS ${CMAKE_THIRD_PARTY_DIR}/paraview)
    endif()

    include_directories(${ParaView_PREFIX_PATH}/include/paraview-${CMAKE_BDM_PVVERSION})
    add_definitions("-DUSE_PARAVIEW")
endif()

if(sbml)
  if(APPLE)
    message(FATAL_ERROR "Currently SBML is not supported on MacOS (see https://trello.com/c/vKPbh4iG).")
  endif()
  find_package(Libroadrunner)
  if(NOT Libroadrunner_FOUND)
    include(external/Libroadrunner)
    find_package(Libroadrunner)
  endif()
endif()

# Load libgit2 if it is enabled
if (libgit2)
    include(external/Libgit2)
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
    find_package(Valgrind)
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

# Link optimlib
include(ExternalProject)
set(OPTIM_LIBNAME ${CMAKE_SHARED_LIBRARY_PREFIX}optim${CMAKE_SHARED_MODULE_SUFFIX})
if(APPLE)
  ExternalProject_Add(
    optim
    URL "${CMAKE_SOURCE_DIR}/third_party/optim-18ae71.zip"
    BINARY_DIR "optim-prefix/src/optim"
    CONFIGURE_COMMAND ./configure -o "-O3 -ffp-contract=fast -flto -DARMA_NO_DEBUG"
    INSTALL_COMMAND cp ${CMAKE_CURRENT_BINARY_DIR}/optim-prefix/src/optim/${OPTIM_LIBNAME} ${CMAKE_INSTALL_LIBDIR}
  )
else()
  ExternalProject_Add(
    optim
    URL "${CMAKE_SOURCE_DIR}/third_party/optim-18ae71.zip"
    BINARY_DIR "optim-prefix/src/optim"
    CONFIGURE_COMMAND ./configure -p -o "-O3 -march=native -ffp-contract=fast -flto -DARMA_NO_DEBUG -DOPTIM_USE_OMP"
    INSTALL_COMMAND cp ${CMAKE_CURRENT_BINARY_DIR}/optim-prefix/src/optim/${OPTIM_LIBNAME} ${CMAKE_INSTALL_LIBDIR}
  )
endif()
add_library(liboptim SHARED IMPORTED)
set_property(TARGET liboptim PROPERTY IMPORTED_LOCATION ${CMAKE_INSTALL_LIBDIR}/${OPTIM_LIBNAME})
include_directories(${CMAKE_CURRENT_BINARY_DIR}/optim-prefix/src/optim/include)
link_directories(${CMAKE_INSTALL_LIBDIR})
set(BDM_REQUIRED_LIBRARIES ${BDM_REQUIRED_LIBRARIES} ${OPTIM_LIBNAME})

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
#  When building, don't use the install RPATH already (but later on when installing)
set(CMAKE_SKIP_BUILD_RPATH FALSE)         # don't skip the full RPATH for the build tree
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE) # use always the build RPATH for the build tree
set(CMAKE_MACOSX_RPATH TRUE)              # use RPATH for MacOSX
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE) # point to directories outside the build tree to the install RPATH

# Check whether to add RPATH to the installation (the build tree always has the RPATH enabled)
if(rpath)
  set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_FULL_LIBDIR}) # install LIBDIR
  set(CMAKE_SKIP_INSTALL_RPATH FALSE)          # don't skip the full RPATH for the install tree
elseif(APPLE)
  set(CMAKE_INSTALL_NAME_DIR "@rpath")
  set(CMAKE_INSTALL_RPATH "@loader_path/../lib")  # self relative LIBDIR
  set(CMAKE_SKIP_INSTALL_RPATH FALSE)          # don't skip the full RPATH for the install tree
else()
  set(CMAKE_BUILD_RPATH_USE_ORIGIN TRUE)       # skip the full RPATH for the install tree
  set(CMAKE_INSTALL_RPATH "\$ORIGIN/../lib")   # self relative LIBDIR
  set(CMAKE_SKIP_INSTALL_RPATH FALSE)          # don't skip the full RPATH for the install tree
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
include_directories("${CMAKE_SOURCE_DIR}/third_party/unibn")
include_directories("${CMAKE_SOURCE_DIR}/third_party/nanoflann")
include_directories("${CMAKE_SOURCE_DIR}/third_party/cxxopts-v2.2.1")
include_directories("${EXTRACTED_THIRD_PARTY_LIBS}")
include_directories("${CMAKE_CURRENT_BINARY_DIR}/version")

# Since including the directory where omp.h resides can cause rootcling to
# search for system / C++ standard headers in that same directory, we instead
# copy omp.h to our local build directory and include that
fix_rootcling_omp_issue()
fix_macos_opencl_header_issue()

# build libbiodynamo
file(GLOB_RECURSE HEADERS "${CMAKE_SOURCE_DIR}/src/*.h")
file(GLOB_RECURSE LIB_SOURCES "${CMAKE_SOURCE_DIR}/src/*.cc")
file(GLOB_RECURSE KERNELS "${CMAKE_SOURCE_DIR}/src/*.cu")

# Exclude paraview-related source files, which will be loaded dynamically as a
# plugin upon request
filter_list(LIB_SOURCES "${LIB_SOURCES}" "paraview/*")
filter_list(HEADERS "${HEADERS}" "paraview/*")

build_shared_library(biodynamo
                   SELECTION selection-libbiodynamo.xml
                   SOURCES ${LIB_SOURCES}
                   HEADERS ${HEADERS}
                   LIBRARIES ${BDM_REQUIRED_LIBRARIES} ${ROOT_LIBRARIES})

# libbiodynamo must be built before optimlib; otherwise optimlib could be missing during
# the linking stage of libbiodynamo
add_dependencies(biodynamo optim)
                  
# Build plugin
if(paraview)
  # Find paraview-related sources and headers for plugin generation
  file(GLOB_RECURSE PV_HEADERS "${CMAKE_SOURCE_DIR}/src/core/visualization/paraview/*.h")
  file(GLOB_RECURSE PV_SOURCES "${CMAKE_SOURCE_DIR}/src/core/visualization/paraview/*.cc")

  # We link against python3 libraries, so that the paraview libraries can
  # resolve missing python symbols when creating binaries
  if(APPLE)
    set(REQUIRED_PARAVIEW_LIBRARIES "${ParaView_LIBRARIES};${Python3_LIBRARIES}")
  else()
    set(REQUIRED_PARAVIEW_LIBRARIES "${ParaView_LIBRARIES};${Python_LIBRARIES}")
  endif()
  build_shared_library(VisualizationAdaptor
                    SELECTION selection-libVisualizationAdaptor.xml
                    SOURCES ${PV_SOURCES}
                    HEADERS ${PV_HEADERS}
                    LIBRARIES ${REQUIRED_PARAVIEW_LIBRARIES} biodynamo
                    PLUGIN "TRUE")
  set(REQUIRED_PARAVIEW_LIBRARIES "${REQUIRED_PARAVIEW_LIBRARIES};VisualizationAdaptor")

  if(LINUX)
    SET_TARGET_PROPERTIES(VisualizationAdaptor PROPERTIES LINK_FLAGS "-Wl,--no-as-needed")
  else()
    # see: https://github.com/pytorch/pytorch/pull/3439/files
    SET_TARGET_PROPERTIES(VisualizationAdaptor PROPERTIES LINK_FLAGS "-dynamic")
  endif()
endif()

if(${VTune_FOUND})
  target_link_libraries(biodynamo ${VTUNE_LIBRARIES})
  add_definitions("-DUSE_VTUNE")
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
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
      message("CUDA Debug mode")
      set(CUDA_NVCC_FLAGS "${CUDA_NVCC_FLAGS} -g -G -O0" )
    endif()
    set(CUDA_NVCC_FLAGS "${CUDA_NVCC_FLAGS} -std=c++17")

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
                         ${CMAKE_SOURCE_DIR}/test/unit/core/analysis/*.cc
                         ${CMAKE_SOURCE_DIR}/test/unit/core/behavior/*.cc
                         ${CMAKE_SOURCE_DIR}/test/unit/core/container/*.cc
                         ${CMAKE_SOURCE_DIR}/test/unit/core/environment/*.cc
                         ${CMAKE_SOURCE_DIR}/test/unit/core/execution_context/*.cc
                         ${CMAKE_SOURCE_DIR}/test/unit/core/memory/*.cc
                         ${CMAKE_SOURCE_DIR}/test/unit/core/operation/*.cc
                         ${CMAKE_SOURCE_DIR}/test/unit/core/multi_simulation/*.cc
                         ${CMAKE_SOURCE_DIR}/test/unit/core/param/*.cc
                         ${CMAKE_SOURCE_DIR}/test/unit/core/agent/*.cc
                         ${CMAKE_SOURCE_DIR}/test/unit/core/util/*.cc
                         ${CMAKE_SOURCE_DIR}/test/unit/core/visualization/*.cc
                         ${CMAKE_SOURCE_DIR}/test/unit/core/visualization/paraview/*.cc
                         ${CMAKE_SOURCE_DIR}/test/unit/neuroscience/*.cc)
  file(GLOB TEST_HEADERS ${CMAKE_SOURCE_DIR}/test/unit/*.h
                         ${CMAKE_SOURCE_DIR}/test/unit/test_util/*.h
                         ${CMAKE_SOURCE_DIR}/test/unit/core/*.h
                         ${CMAKE_SOURCE_DIR}/test/unit/core/analysis/*.h
                         ${CMAKE_SOURCE_DIR}/test/unit/core/behavior/*.h
                         ${CMAKE_SOURCE_DIR}/test/unit/core/container/*.h
                         ${CMAKE_SOURCE_DIR}/test/unit/core/environment/*.h
                         ${CMAKE_SOURCE_DIR}/test/unit/core/execution_context/*.h
                         ${CMAKE_SOURCE_DIR}/test/unit/core/memory/*.h
                         ${CMAKE_SOURCE_DIR}/test/unit/core/operation/*.h
                         ${CMAKE_SOURCE_DIR}/test/unit/core/param/*.h
                         ${CMAKE_SOURCE_DIR}/test/unit/core/operation/*.h
                         ${CMAKE_SOURCE_DIR}/test/unit/core/agent/*.h
                         ${CMAKE_SOURCE_DIR}/test/unit/core/util/*.h
                         ${CMAKE_SOURCE_DIR}/test/unit/core/visualization/*.h
                         ${CMAKE_SOURCE_DIR}/test/unit/core/visualization/paraview/*.h
                         ${CMAKE_SOURCE_DIR}/test/unit/neuroscience/*.h)
  
  bdm_add_test_executable(biodynamo-unit-tests
                          SOURCES ${TEST_SOURCES}
                          HEADERS ${TEST_HEADERS}
                          LIBRARIES ${REQUIRED_PARAVIEW_LIBRARIES})

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
endif()

if(OPENCL_FOUND OR CUDA_FOUND)
  bdm_add_executable(cell_division_gpu
                      SOURCES test/system/cell_division_gpu/src/cell_division_gpu.cc
                      HEADERS test/system/cell_division_gpu/src/cell_division_gpu.h
                      LIBRARIES biodynamo)
endif()

# -------------------- benchmarking --------------------------------------------
if(benchmark)
  include(Benchmarking)
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

#---Populate the configure arguments returned by 'bdm-config --config'
get_cmake_property(variables CACHE_VARIABLES)
foreach(var ${variables})
  if((var MATCHES "_(LIBRARIES|LIBRARY|INCLUDE|VERSION)") AND
     (NOT ${${var}} STREQUAL "") AND
     (NOT ${var} MATCHES "NOTFOUND"))
    if (var MATCHES "^QT_")
      # filter out the very long list of Qt libraries and include dirs
      if (var MATCHES "(QT_LIBRARY_DIR|QT_QTCORE_INCLUDE_DIR)")
        set(BDM_CONFIGARGS "${BDM_CONFIGARGS}${var}=${${var}} ")
      endif()
    else()
      if ((NOT var MATCHES "_(DOCS|TESTS|INSTALL)") AND (NOT var MATCHES "^_"))
        set(BDM_CONFIGARGS "${BDM_CONFIGARGS}${var}=${${var}} ")
      endif()
    endif()
  endif()
endforeach()

#--Local bdm-config and thisbdm
EXECUTE_PROCESS(COMMAND ${ROOT_CONFIG_EXECUTABLE} --arch COMMAND tr -d '\n' OUTPUT_VARIABLE architecture)
EXECUTE_PROCESS(COMMAND ${ROOT_CONFIG_EXECUTABLE} --platform COMMAND tr -d '\n' OUTPUT_VARIABLE platform)
EXECUTE_PROCESS(COMMAND ${ROOT_CONFIG_EXECUTABLE} --version COMMAND tr -d '\n' OUTPUT_VARIABLE rootvers)
EXECUTE_PROCESS(COMMAND ${ROOT_CONFIG_EXECUTABLE} --ncpu COMMAND tr -d '\n' OUTPUT_VARIABLE ncpu)
get_property(cxxdefinesraw DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY COMPILE_DEFINITIONS)

set(altcxx ${CMAKE_CXX_COMPILER})
set(altld ${CMAKE_CXX_COMPILER})
set(cxxflags ${CMAKE_CXX_FLAGS})
set(cxxstd ${CMAKE_CXX_STANDARD})
set(cxxflagsdebug ${CMAKE_CXX_FLAGS_DEBUG})
set(cxxflagsrelease ${CMAKE_CXX_FLAGS_RELEASE})
foreach(definition ${cxxdefinesraw})
  set(cxxdefines "${cxxdefines} -D${definition}")
endforeach()
set(ldflags ${CMAKE_EXE_LINKER_FLAGS})
set(soflags ${CMAKE_SHARED_LINKER_FLAGS})
set(socreatecxxflags ${CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS})
set(bindir "$BDMSYS/bin")
set(libdir "$BDMSYS/lib")
set(incdir "$BDMSYS/include")
set(cmakedir "$BDMSYS/share/cmake")
set(configargs ${BDM_CONFIGARGS})
set(cmakeinvoke "${CMAKE_INVOKE} -G ${CMAKE_GENERATOR}")
set(omplibs ${OpenMP_CXX_LIBRARIES})
set(with_pyenv ON)
if (NOPYENV)
   set(with_pyenv OFF)
endif()
if (DEFINED ENV{PYENV_ROOT})
  set(pyenvroot "$ENV{PYENV_ROOT}")
else()
  set(pyenvroot "")
endif()
set(with_paraview ON)
if(NOT paraview)
  set(with_paraview OFF)
endif()

# Copy the environmental files to their correct directory
configure_file(${CMAKE_SOURCE_DIR}/cmake/env/thisbdm.sh ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/thisbdm.sh @ONLY NEWLINE_STYLE UNIX)
configure_file(${CMAKE_SOURCE_DIR}/cmake/env/thisbdm.fish ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/thisbdm.fish @ONLY NEWLINE_STYLE UNIX)
configure_file(${CMAKE_SOURCE_DIR}/cmake/env/bdm-config.in ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/bdm-config @ONLY ESCAPE_QUOTES NEWLINE_STYLE UNIX)

# Set run permission on the environmental files
add_permissions("${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/thisbdm.sh" ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_permissions("${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/thisbdm.fish" ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_permissions("${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/bdm-config" ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# Copy over sh_functions directory and contents, and set execute permissions
file(GLOB _SHELL_FNS ${CMAKE_SOURCE_DIR}/cmake/env/sh_functions/*)
file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/sh_functions)
file(COPY ${_SHELL_FNS} DESTINATION  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/sh_functions
        FILE_PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE)
        
if (${CMAKE_GENERATOR} STREQUAL "Unix Makefiles")
  set(buildcmd "make -j ${ncpu}")
elseif (${CMAKE_GENERATOR} STREQUAL "Ninja")
  set(buildcmd "ninja")
else()
  set(buildcmd "cmake --build . -j ${ncpu}")
endif()

PRINT_LINE()
MESSAGE("You have just finished to configure the BioDynaMo source.\n\
Compile it by running \"${buildcmd}\".\n\
Before using it do \". ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/thisbdm.sh\".")
PRINT_LINE()
