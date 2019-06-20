cmake_minimum_required(VERSION 3.2.0)
# With CMAKE >= 3.2 external projects support byproducts, this is needed
# to support Ninja as generator

project(BioDynaMo)
set(BDM_VERSION 0.1.0)

# Options. Turn on with 'cmake -Dmyvarname=ON'.
option(test      "Build all tests." ON) # Makes boolean 'test' available.
option(cuda      "Enable CUDA code generation for GPU acceleration" OFF)
option(opencl    "Enable OpenCL code generation for GPU acceleration" OFF)
option(dict      "Build with ROOT dictionaries" ON)
option(paraview  "Enable ParaView" ON)
option(vtune     "Enable VTune performance analysis" OFF)
option(coverage "Enable test coverage report generation. Sets build type to coverage" OFF)

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

# Directory in which BioDynaMo will be installed. It can be provided
# by the user itself.
if (NOT DEFINED ${BDM_INSTALL_DIR})
    set(BDM_INSTALL_DIR $ENV{HOME}/.bdm)
endif()

# Directory in which all the third party code will be stored
# Directory in which we saved all the cmake informations.
set(THIRD_PARTY_DIR ${BDM_INSTALL_DIR}/third_party)
set(BDM_CMAKE_DIR ${CMAKE_SOURCE_DIR}/cmake)

# Add an additional path to make include() look in the right places.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

# Include some utils functions (print_line, print_warning, verify_root, etc.)
include(utils)

# Detect os type and version
detect_os()

## TODO(lukas) add logic to detect correct env script (distinguishing LINUX and
## APPLE might not be enough in the future)
if(LINUX)
  configure_file(util/installation/common/biodynamo-linux-env.sh ${CMAKE_CURRENT_BINARY_DIR}/biodynamo-env.sh @ONLY)
elseif(APPLE)
  configure_file(util/installation/common/biodynamo-macos-env.sh ${CMAKE_CURRENT_BINARY_DIR}/biodynamo-env.sh @ONLY)
endif()

# Check if there was a previously installed version of BioDynaMo. If that's the case, then update the biodynamo-env
# file and just tell the user.
execute_process(COMMAND ${CMAKE_COMMAND} -E compare_files "${CMAKE_CURRENT_BINARY_DIR}/biodynamo-env.sh" "${BDM_INSTALL_DIR}/biodynamo-env.sh" RESULT_VARIABLE ret)
if(ret EQUAL "1")
    file(COPY "${CMAKE_CURRENT_BINARY_DIR}/biodynamo-env.sh" DESTINATION "${BDM_INSTALL_DIR}")
    print_warning()
    message( "We found another BioDynaMo environment file ($ENV{BDM_INSTALL_DIR}/biodynamo-env.sh). However, it is outdated. \
We have updated the file automatically.")
    print_line()

endif()

add_definitions(-DBDM_SRC_DIR=\"${CMAKE_SOURCE_DIR}/src\")  # escaped quotes are mandatory

# -------------------- find packages ------------------------------------------
# CMake cannot find LLVM's Clang OpenMP library by default (this makes
# find_package(OpenMP) fail). Therefore we manually specify the OpenMP flags.
# Furthermore, rootcling cannot find omp.h by default, so we copy this (with a
# shell script to /usr/local/Cellar/biodynamo, which is brew's default install
# directory), and include this directory. We cannot directly include the
# original directory, because of header conflicts (such as stdint.h)
if(APPLE)
  link_directories(/usr/local/opt/llvm/lib)
  set(OpenMP_C_FLAGS -fopenmp=libomp)
  set(OpenMP_CXX_FLAGS -fopenmp=libomp)
  include_directories(/usr/local/Cellar/biodynamo)
else()
  find_package(OpenMP REQUIRED)
endif()
if (test)
  # must be before include(${ROOT_USE_FILE}) otherwise test compile fails
  find_package(Threads REQUIRED)
endif()

if(cuda)
  find_package(CUDA)
endif()
if(opencl)
  find_package(OpenCL)
endif()
find_package(Git)

# Find ROOT and check its installation
find_package(ROOT COMPONENTS Geom Gui)
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

find_package(Numa REQUIRED)
if (NUMA_FOUND)
   include_directories(${NUMA_INCLUDE_DIR})
   link_directories(${NUMA_LIBRARY_DIR})
   add_definitions("-DUSE_NUMA")
   set(BDM_REQUIRED_LIBRARIES ${BDM_REQUIRED_LIBRARIES} ${NUMA_LIBRARIES})
endif()

if(paraview)
  find_package(ParaView 5.3 QUIET OPTIONAL_COMPONENTS vtkPVPythonCatalyst
    vtkPVVTKExtensionsDefault vtkPVServerManagerRendering)
  if(${ParaView_FOUND})
    message(STATUS "Found ParaView")
    include("${PARAVIEW_USE_FILE}")
    add_definitions("-DUSE_CATALYST")
    link_directories($ENV{ParaView_LIB_DIR})
    set(BDM_REQUIRED_LIBRARIES ${BDM_REQUIRED_LIBRARIES} vtkPVPythonCatalyst
        vtkIOParallelXML vtkPVVTKExtensionsDefault vtkPVServerManagerRendering)
  else()
    message(WARNING "ParaView not found")
  endif()
endif()

# paraview version 5.5 (MacOS) does not contain TBB
if (APPLE OR (NOT paraview OR NOT ${ParaView_FOUND}))
  find_package(TBB REQUIRED)
  if (TBB_FOUND)
    include_directories(${TBB_INCLUDE_DIRS})
    link_directories(${TBB_LIBRARIES})
    set(BDM_REQUIRED_LIBRARIES ${BDM_REQUIRED_LIBRARIES} ${TBB_LIBRARIES})
  endif()
endif()

if (vtune)
    find_package(VTune)
    if(${VTune_FOUND})
        include_directories(${VTUNE_INCLUDE_DIR})
        add_definitions("-DUSE_VTUNE")
        set(BDM_REQUIRED_LIBRARIES ${BDM_REQUIRED_LIBRARIES} ${VTUNE_LIBRARIES})
    else()
        message(WARNING "VTune not found")
    endif()
endif()

# -------------------- includes -----------------------------------------------
include(BioDynaMo)
include(ExternalProject)
include(Documentation)
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
set(CMAKE_SKIP_BUILD_RPATH FALSE) # don't skip the full RPATH for the build tree
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

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
include_directories("${EXTRACTED_THIRD_PARTY_LIBS}")
include_directories("${CMAKE_CURRENT_BINARY_DIR}/version")

fix_rootcling_omp_issue()

# build libbiodynamo
file(GLOB_RECURSE HEADERS "${CMAKE_SOURCE_DIR}/src/*.h")
file(GLOB_RECURSE LIB_SOURCES "${CMAKE_SOURCE_DIR}/src/*.cc")
file(GLOB_RECURSE KERNELS "${CMAKE_SOURCE_DIR}/src/*.cu")
build_libbiodynamo(biodynamo
                   SOURCES ${LIB_SOURCES}
                   HEADERS ${HEADERS}
                   LIBRARIES ${BDM_REQUIRED_LIBRARIES} ${ROOT_LIBRARIES})
if(${VTune_FOUND})
  target_link_libraries(biodynamo ${VTUNE_LIBRARIES})
endif()

if (NOT ROOT_FOUND)
    add_dependencies(${BIODYNAMO_TARGET_NAME} ROOT)
endif()

# Check if OpenCL C++ bindings are installed
if (OPENCL_FOUND)
  if (NOT OPENCL_HAS_CXX)
    message(FATAL_ERROR "OpenCL C++ bindings not found. Please install to make use of OpenCL. "
      "If you think you have installed the C++ bindings correctly, please check if one "
      "of the following environmentals is set correctly (vendor specific):
      - AMD: \t\tAMDAPPSDKROOT
      - NVIDIA: \tCUDA_PATH
      - INTEL: \tINTELOPENCLSDK")
  endif()
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
  build_paraview_plugin()
endif()

# create cmake files used by external builds
configure_file("cmake/UseBioDynaMo.cmake.in" "UseBioDynaMo.cmake" @ONLY)

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
  bdm_add_test_executable(runBiodynamoTestsMain
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

  if (NOT ROOT_FOUND)
      add_dependencies(${BIODYNAMO_TEST_TARGET_NAME} ROOT)
  endif()

  # integration tests
  if (NOT coverage)
    if(OPENCL_FOUND OR CUDA_FOUND)
      bdm_add_executable(cell_division_gpu
                         SOURCES test/system/cell_division_gpu.cc
                         HEADERS test/system/cell_division_gpu.h
                         LIBRARIES biodynamo)
      add_dependencies(check cell_division_gpu)
      add_test(NAME "system-cell-division-gpu"
               COMMAND ${CMAKE_SOURCE_DIR}/build/cell_division_gpu)
    endif()
  endif()
endif()

# -------------------- Install configuration -----------------------------------
include(Installation)

# -------------------- add targets for code style and submission checks --------
include(CppStyleGuideChecks)

# --------------------- Final Message to the user ------------------------------

PRINT_LINE()
MESSAGE("You have just finished to configure BioDynaMo. Now you can run \"make .\" to compile it.\n\
Remember to source ${BDM_INSTALL_DIR}/biodynamo-env.sh before actually running the tests or using\n\
the library.")
PRINT_LINE()