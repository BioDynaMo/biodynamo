# llvm-13
# CMake might not expand '~' in HINTS reliably.
# It's safer to use an absolute path or set an environment variable
# and use $ENV{...} or pass it as a CMake variable -DLLVM_ROOT=...
set(MY_LLVM_INSTALL_DIR "$ENV{HOME}/buildroadrunner/llvm-13.x/install-msvc2019") # Or provide the absolute path directly

find_package(LLVM 13 REQUIRED CONFIG
             HINTS "${MY_LLVM_INSTALL_DIR}/lib/cmake/llvm"  # Common location
                   "${MY_LLVM_INSTALL_DIR}/lib64/cmake/llvm" # Another common location
                   "${MY_LLVM_INSTALL_DIR}" # Fallback if cmake files are directly in the root (less common for LLVM)
)

# Check if LLVM was found and print the version
if(LLVM_FOUND)
  message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
  message(STATUS "Using LLVM_INCLUDE_DIRS: ${LLVM_INCLUDE_DIRS}")
  message(STATUS "Using LLVM_LIBRARY_DIRS: ${LLVM_LIBRARY_DIRS}") # Might be useful to check
else()
  message(FATAL_ERROR "LLVM 13 not found. Please check MY_LLVM_INSTALL_DIR and HINTS.")
endif()

include_directories(SYSTEM ${LLVM_INCLUDE_DIRS}) # Use SYSTEM for system/external headers
add_definitions(${LLVM_DEFINITIONS})

llvm_map_components_to_libnames(LLVM_LIBS
    Core
    Analysis
    Target
    CodeGen
    ScalarOpts
    InstCombine
    TransformUtils
    BitReader
    BitWriter
    IRReader
    Object
    MCJIT
    ExecutionEngine
    RuntimeDyld
    LTO
    Support
    Demangle
    JITLink
    OrcJIT
    OrcShared
    OrcTargetProcess

    # Architecture-specific components for ARM64 (macOS)
    AArch64CodeGen
    AArch64Info
    AArch64AsmParser
    AArch64Utils
)

# roadrunner
set(MY_ROADRUNNER_INSTALL_DIR "$ENV{HOME}/buildroadrunner/roadrunner/install-Release")

find_library(Libroadrunner_LIBRARY
  NAMES roadrunner roadrunner-static
  HINTS "${MY_ROADRUNNER_INSTALL_DIR}/lib"
        "${MY_ROADRUNNER_INSTALL_DIR}/lib64"
        "${MY_ROADRUNNER_INSTALL_DIR}"
        ${CMAKE_THIRD_PARTY_DIR}/libroadrunner/lib
        $ENV{BDMSYS}/third_party/libroadrunner/lib
)

if (Libroadrunner_LIBRARY)
  set(Libroadrunner_FOUND TRUE)
  message(STATUS "Found Libroadrunner: ${Libroadrunner_LIBRARY}")
  message(STATUS "Libroadrunner install directory appears to be: ${MY_ROADRUNNER_INSTALL_DIR}") # For confirmation
  set(LIBRR_INSTALL_DIR ${MY_ROADRUNNER_INSTALL_DIR})


  set(Libroadrunner_INCLUDE_DIRS ${LLVM_INCLUDE_DIRS}
                              ${LIBRR_INSTALL_DIR}/include
                              ${LIBRR_INSTALL_DIR}/include/rr
                              ${LIBRR_INSTALL_DIR}/include/sbml
                              ${LIBRR_INSTALL_DIR}/include/cvode)
  message(STATUS "Libroadrunner_INCLUDE_DIRS: ${Libroadrunner_INCLUDE_DIRS}")

  set(Libroadrunner_LINK_DIRS ${LIBRR_INSTALL_DIR}/lib
                              ${LIBRR_INSTALL_DIR}/lib64
                              ${LLVM_LIBRARY_DIRS})
  message(STATUS "Libroadrunner_LINK_DIRS: ${Libroadrunner_LINK_DIRS}")

  set(Libroadrunner_DEFINITIONS ${LLVM_DEFINITIONS})
  set(Libroadrunner_INCLUDE_DIRS ${LLVM_INCLUDE_DIRS}
                              ${LIBRR_INSTALL_DIR}/include
                              ${LIBRR_INSTALL_DIR}/include/rr
                              ${LIBRR_INSTALL_DIR}/include/sbml
                              ${LIBRR_INSTALL_DIR}/include/cvode)
  set(Libroadrunner_LINK_DIRS ${LIBRR_INSTALL_DIR}/lib
                              ${LIBRR_INSTALL_DIR}/lib64)

  set(Libroadrunner_LINK_LIBRARIES
    roadrunner-static
    nleq1-static.a
    nleq2-static.a
    rr-libstruct-static
    lapack
    blas
    f2c
    libsbml-static.a
    xml2

    sundials_cvodes.a
    sundials_kinsol.a
    sundials_nvecserial.a
    sundials_sunmatrixdense.a
    sundials_sunlinsoldense.a
    sundials_sunnonlinsolfixedpoint.a
    sundials_sunnonlinsolnewton.a
    sundials_core.a

    pthread
    dl
    z
    PocoNet PocoXML PocoFoundation
    bz2
  )
  set(Libroadrunner_LINK_LIBRARIES ${Libroadrunner_LINK_LIBRARIES} ${LLVM_LIBS})

else()
  if (Libroadrunner_FIND_REQUIRED)
    message(FATAL_ERROR "Libroadrunner not found")
  else()
    message(STATUS "Libroadrunner not found")
  endif()
endif()
