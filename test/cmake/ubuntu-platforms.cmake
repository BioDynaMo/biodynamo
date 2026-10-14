# Run with: cmake -P test/cmake/ubuntu-platforms.cmake
cmake_minimum_required(VERSION 3.19)
set(CMAKE_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../..")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(utils)
include(external/SHA256Digests)
set(APPLE FALSE)

foreach(os ubuntu-22.04 ubuntu-24.04 ubuntu-26.04)
  set(DETECTED_OS "${os}")
  set(DETECTED_OS_VERS "${os}")
  set(DETECTED_ARCH x86_64)
  check_detected_os("${os}")
  set(expected_os "${os}")
  if(os STREQUAL "ubuntu-26.04")
    set(expected_os ubuntu-24.04)
  endif()

  bdm_third_party_os(package_os)
  bdm_root_platform(root_tar root_key)
  bdm_paraview_platform(pv_tar pv_key)
  if(NOT package_os STREQUAL expected_os OR
     NOT root_tar STREQUAL "root_v6.30.02_cxx17_python3.9_${expected_os}.tar.gz" OR
     NOT pv_tar STREQUAL "paraview_v5.9.0_${expected_os}_default.tar.gz" OR
     NOT root_key STREQUAL "${os}-ROOT" OR
     NOT pv_key STREQUAL "${os}-ParaView")
    message(FATAL_ERROR "Wrong archive or checksum selection for ${os}")
  endif()
endforeach()

# Reused binaries must retain their original checksums for every dependency.
foreach(package ROOT ParaView Qt Libroadrunner)
  if(NOT "${ubuntu-26.04-${package}}" STREQUAL "${ubuntu-24.04-${package}}")
    message(FATAL_ERROR "Ubuntu 26.04 reuses 24.04 but ${package} checksums differ")
  endif()
endforeach()

# An unsupported architecture must never silently use x86_64 archives.
set(DETECTED_OS_VERS ubuntu-26.04-aarch64)
bdm_third_party_os(package_os)
if(NOT package_os STREQUAL "ubuntu-26.04-aarch64")
  message(FATAL_ERROR "Architecture was lost when choosing third-party archives")
endif()
message(STATUS "Ubuntu third-party platform selection passed")
