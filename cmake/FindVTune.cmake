# from VecGeom

if ($ENV{VTUNE_DIR})
  set(VTUNE_DIR $ENV{VTUNE_DIR})
endif()

if (NOT VTUNE_DIR)
  set(VTUNE_DIR /opt/intel/vtune_amplifier_xe/)
endif()

if (MIC)
  set(VTUNE_LIB_DIR ${VTUNE_DIR}/bin64/k1om/)
else()
  set(VTUNE_LIB_DIR ${VTUNE_DIR}/lib64)
endif()

find_library(VTUNE_LIBRARIES ittnotify PATHS "${VTUNE_LIB_DIR}" NO_DEFAULT_PATH)

if (VTUNE_LIBRARIES)
  set(VTune_FOUND TRUE)
  set(VTUNE_INCLUDE_DIR "${VTUNE_DIR}/include")
  message(STATUS "Found Vtune library ${VTUNE_LIBRARIES}")
else()
  message(STATUS "Vtune library not found; try to set a VTUNE_DIR environment variable to the base installation path or add -DVTUNE_DIR to the cmake command")
endif()
