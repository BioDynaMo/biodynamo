cmake_minimum_required(VERSION 3.2.0)
# With CMAKE >= 3.2 external projects support byproducts, this is needed
# to support Ninja as generator

project(BioDynaMo)
set(BDM_VERSION 0.1.0)

# Options. Turn on with 'cmake -Dmyvarname=ON'.
option(test "Build all tests." ON) # Makes boolean 'test' available.
option(demo "Build all demos." ON)
option(cuda "Enable CUDA code generation for GPU acceleration" OFF)
option(opencl "Build OpenCL code generation for GPU acceleration" OFF)
if(APPLE)
   # valgrind currently broken on macOS 10.12
   option(valgrind "Enable valgrind tests and make build compatible with valgrind tool." OFF)
else()
   option(valgrind "Enable valgrind tests and make build compatible with valgrind tool." ON)
endif()
option(coverage "Enable test coverage report generation. Sets build type to coverage" OFF)

# detect if the biodynamo environment has been sourced
if(NOT DEFINED ENV{BDM_CMAKE_DIR})
  execute_process(COMMAND rm -rf ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles)
  message(FATAL_ERROR "The BioDynaMo environment is not set up correctly. Please call 'source <path-to-bdm-installation>/biodynamo-env.sh' and retry this command.")
endif()

set(BDM_CMAKE_DIR cmake)

if(UNIX AND NOT APPLE)
  set(LINUX TRUE)
endif()

add_definitions(-DBDM_SRC_DIR=\"${CMAKE_SOURCE_DIR}/src\")  # escaped quotes are mandatory

# -------------------- find packages ------------------------------------------
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
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
find_package(ROOT REQUIRED COMPONENTS Geom Gui)

find_package(ClangTools)
if ("$ENV{CMAKE_EXPORT_COMPILE_COMMANDS}" STREQUAL "1" OR CLANG_TIDY_FOUND)
  # Generate a Clang compile_commands.json "compilation database" file for use
  # with various development tools, such as Vim's YouCompleteMe plugin.
  # See http://clang.llvm.org/docs/JSONCompilationDatabase.html
  set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
endif()

find_package(ParaView 5.3 QUIET OPTIONAL_COMPONENTS vtkPVPythonCatalyst
  vtkPVVTKExtensionsDefault vtkPVServerManagerRendering)
if(${ParaView_FOUND})
  message(STATUS "Found ParaView")
  include("${PARAVIEW_USE_FILE}")
  add_definitions("-DUSE_CATALYST")
else()
  message(WARNING "ParaView not found")
endif()

find_package(VTune)
if(${VTune_FOUND})
  include_directories(${VTUNE_INCLUDE_DIR})
  add_definitions("-DUSE_VTUNE")
else()
  message(WARNING "VTune not found")
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

# -------------------- setup mpark variant library -----------------------------
set(MPARK_LIB "${CMAKE_CURRENT_BINARY_DIR}/mpark/")
file(MAKE_DIRECTORY "${MPARK_LIB}/")
execute_process(COMMAND ${CMAKE_COMMAND} -E tar xf "${CMAKE_SOURCE_DIR}/third_party/mpark_4ee0086.zip"
                WORKING_DIRECTORY "${MPARK_LIB}")

# -------------------- build shared library libbiodynamo -----------------------
# add the headers into the project
include(${ROOT_USE_FILE})
include_directories("${CMAKE_SOURCE_DIR}/src")
include_directories("${CMAKE_SOURCE_DIR}/test")
include_directories("${CMAKE_SOURCE_DIR}/third_party")
include_directories("${CMAKE_SOURCE_DIR}/demo")
include_directories("${MPARK_LIB}")
include_directories("${CMAKE_CURRENT_BINARY_DIR}/version")

# Fix for rootcling not able to find omp.h. We cannot include the entire include
# directory of the compiler (where omp.h is in), because rootcling starts using
# the intrinsics found in there. Therefore we do a local copy into the build dir.
execute_process(COMMAND ${CMAKE_CXX_COMPILER} --print-file-name=include/omp.h
                OUTPUT_VARIABLE OMP_HEADER_PATH)
# above command returns path with "\n" appended
string(REGEX REPLACE "\n$" "" OMP_HEADER_PATH "${OMP_HEADER_PATH}")
execute_process(COMMAND mkdir -p ${CMAKE_SOURCE_DIR}/build/omp)
execute_process(COMMAND cp ${OMP_HEADER_PATH} ${CMAKE_SOURCE_DIR}/build/omp)
include_directories("${CMAKE_SOURCE_DIR}/build/omp")

# build libbiodynamo
file(GLOB_RECURSE HEADERS "${CMAKE_SOURCE_DIR}/src/*.h")
file(GLOB_RECURSE LIB_SOURCES "${CMAKE_SOURCE_DIR}/src/*.cc")
file(GLOB_RECURSE KERNELS "${CMAKE_SOURCE_DIR}/src/*.cu")
build_libbiodynamo(biodynamo
                   SOURCES ${LIB_SOURCES}
                   HEADERS ${HEADERS}
                   LIBRARIES ${ROOT_LIBRARIES})
if(${VTune_FOUND})
  target_link_libraries(biodynamo ${VTUNE_LIBRARIES})
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
  link_directories($ENV{ParaView_LIB_DIR})
  target_link_libraries(biodynamo vtkPVPythonCatalyst vtkIOParallelXML
    vtkPVVTKExtensionsDefault vtkPVServerManagerRendering)
endif()

# create cmake files used by external builds
configure_file("cmake/UseBioDynaMo.cmake.in" "UseBioDynaMo.cmake" @ONLY)

build_paraview_plugin()

# -------------------- build test project  ------------------------------------
if (test)
  enable_testing()

  include(Testing)

  # unit tests
  #   main test executable
  file(GLOB TEST_SOURCES ${CMAKE_SOURCE_DIR}/test/unit/*.cc
                         ${CMAKE_SOURCE_DIR}/test/unit/biology_module/*.cc
                         ${CMAKE_SOURCE_DIR}/test/unit/neuroscience/*.cc)
  file(GLOB TEST_HEADERS ${CMAKE_SOURCE_DIR}/test/unit/*.h
                         ${CMAKE_SOURCE_DIR}/test/unit/biology_module/*.h
                         ${CMAKE_SOURCE_DIR}/test/unit/neuroscience/*.h)
  bdm_add_test_executable(runBiodynamoTestsMain
                          SOURCES ${TEST_SOURCES}
                          HEADERS ${TEST_HEADERS})

  #   separate test binaries - each source file is compiled into a separate binary
  file(GLOB TEST_SOURCES ${CMAKE_SOURCE_DIR}/test/unit/separate_binary/*.cc)
  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/empty_header.h "")
  foreach(file ${TEST_SOURCES})
    get_filename_component(filename ${file} NAME_WE)
    set(TEST_HEADER ${CMAKE_SOURCE_DIR}/test/unit/separate_binary/${filename}.h)
    if (NOT EXISTS ${TEST_HEADER})
      set(TEST_HEADER ${CMAKE_CURRENT_BINARY_DIR}/empty_header.h)
    endif()
    bdm_add_test_executable("runBiodynamoTests_${filename}"
                            SOURCES ${file}
                            HEADERS ${TEST_HEADER})
  endforeach()

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
