
# llvm-6
find_package(LLVM 6 REQUIRED CONFIG
             HINTS "/opt/rh/llvm-toolset-6.0/root/usr/lib64/cmake/llvm")
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
# Find the libraries that correspond to the LLVM components
# that we wish to use
llvm_map_components_to_libnames(LLVM_LIBS mcjit native)

# roadrunner
find_library(Libroadrunner_LIBRARY
  NAMES roadrunner
  HINTS ${CMAKE_THIRD_PARTY_DIR}/libroadrunner/lib
        $ENV{BDMSYS}/third_party/libroadrunner/lib
)

if (Libroadrunner_LIBRARY)
  set(Libroadrunner_FOUND TRUE)
  message(STATUS "Found Libroadrunner: ${Libroadrunner_LIBRARY}")

  get_filename_component(Libroadrunner_LIB_DIR ${Libroadrunner_LIBRARY} DIRECTORY)
  get_filename_component(LIBRR_INSTALL_DIR "${Libroadrunner_LIB_DIR}/.." ABSOLUTE)

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
    sundials_nvecserial.a
    sundials_cvode.a
    pthread
    dl
    z
    PocoUtil PocoNet PocoXML PocoFoundation
    bz2
    tinfo
  )
  set(Libroadrunner_LINK_LIBRARIES ${Libroadrunner_LINK_LIBRARIES} ${LLVM_LIBS})

else()
  if (Libroadrunner_FIND_REQUIRED)
    message(FATAL_ERROR "Libroadrunner not found")
  else()
    message(STATUS "Libroadrunner not found")
  endif()
endif()
