# "Calling Doxygen from cmake" from Thomas Pircher
# used under a Creative Commons Attribution-Share Alike 3.0 Unported License
# https://creativecommons.org/licenses/by-sa/3.0/
# Changes where made; original article can be found at:
# https://www.tty1.net/blog/2014/cmake-doxygen_en.html

# add a target to generate API documentation with Doxygen
find_package(Doxygen)

if(DOXYGEN_FOUND)
  # create output directory
  file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/doc")

  set(doxyfile_in "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Doxyfile.in")
  set(doxyfile "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile")

  configure_file(${doxyfile_in} ${doxyfile} @ONLY)

  add_custom_target(doc
      # ALL
      COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
      WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
      COMMENT "Generating API documentation with Doxygen.
       Open the following file in your browser: ${CMAKE_BINARY_DIR}/doc/html/index.html"
      VERBATIM)

  # Issue with long file names when building deb pacakages
  # https://gitlab.kitware.com/cmake/cmake/issues/14332
  # Issue resolved in CMake 3.7.2
  # install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/doc/html" DESTINATION share/doc)
endif()
