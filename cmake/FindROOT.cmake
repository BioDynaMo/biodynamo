# - Finds ROOT installation
# This module sets up ROOT information
# It defines:
# ROOT_FOUND             If the ROOT is found
# ROOT_INCLUDE_DIR       PATH to the include directory
# ROOT_INCLUDE_DIRS      PATH to the include directories (not cached)
# ROOT_LIBRARIES         Most common libraries
# ROOT_<name>_LIBRARY    Full path to the library <name>
# ROOT_LIBRARY_DIR       PATH to the library directory
# ROOT_ETC_DIR           PATH to the etc directory
# ROOT_DEFINITIONS       Compiler definitions
# ROOT_CXX_FLAGS         Compiler flags to be used by client packages
# ROOT_C_FLAGS           Compiler flags to be used by client packages
# ROOT_EXE_LINKER_FLAGS  Linker flags to be used by client packages
#
# Updated by K. Smith (ksmith37@nd.edu) to properly handle
#  dependencies in ROOT_GENERATE_DICTIONARY

find_program(ROOT_CONFIG_EXECUTABLE NAMES root-config
  HINTS "$ENV{ROOTSYS}/bin" "$ENV{BDM_ROOT_DIR}/bin" "${CMAKE_THIRD_PARTY_DIR}/root/bin")

execute_process(
    COMMAND ${ROOT_CONFIG_EXECUTABLE} --prefix
    OUTPUT_VARIABLE ROOTSYS
    OUTPUT_STRIP_TRAILING_WHITESPACE)

execute_process(
    COMMAND ${ROOT_CONFIG_EXECUTABLE} --version
    OUTPUT_VARIABLE ROOT_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE)

execute_process(
    COMMAND ${ROOT_CONFIG_EXECUTABLE} --incdir
    OUTPUT_VARIABLE ROOT_INCLUDE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE)
set(ROOT_INCLUDE_DIRS ${ROOT_INCLUDE_DIR})

execute_process(
    COMMAND ${ROOT_CONFIG_EXECUTABLE} --etcdir
    OUTPUT_VARIABLE ROOT_ETC_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE)
set(ROOT_ETC_DIRS ${ROOT_ETC_DIR})

execute_process(
    COMMAND ${ROOT_CONFIG_EXECUTABLE} --libdir
    OUTPUT_VARIABLE ROOT_LIBRARY_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE)
set(ROOT_LIBRARY_DIRS ${ROOT_LIBRARY_DIR})

set(rootlibs Core RIO Net Hist Graf Graf3d Gpad Tree Rint Postscript Matrix Physics MathCore Thread MultiProc Imt)
set(ROOT_LIBRARIES)
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

execute_process(
    COMMAND ${ROOT_CONFIG_EXECUTABLE} --cflags
    OUTPUT_VARIABLE __cflags
    OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REGEX MATCHALL "-(D|U)[^ ]*" ROOT_DEFINITIONS "${__cflags}")
string(REGEX REPLACE "(^|[ ]*)-I[^ ]*" "" ROOT_CXX_FLAGS "${__cflags}")
string(REGEX REPLACE "(^|[ ]*)-I[^ ]*" "" ROOT_C_FLAGS "${__cflags}")

execute_process(
    COMMAND ${ROOT_CONFIG_EXECUTABLE} --ldflags
    OUTPUT_VARIABLE __ldflags
    OUTPUT_STRIP_TRAILING_WHITESPACE)
set(ROOT_EXE_LINKER_FLAGS "${__ldflags}")

set(ROOT_USE_FILE ${CMAKE_CURRENT_LIST_DIR}/RootUseFile.cmake)

execute_process(
  COMMAND ${ROOT_CONFIG_EXECUTABLE} --features
  OUTPUT_VARIABLE _root_options
  OUTPUT_STRIP_TRAILING_WHITESPACE)
separate_arguments(_root_options)
foreach(_opt ${_root_options})
  set(ROOT_${_opt}_FOUND TRUE)
endforeach()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ROOT DEFAULT_MSG ROOT_CONFIG_EXECUTABLE
    ROOTSYS ROOT_VERSION ROOT_INCLUDE_DIR ROOT_LIBRARIES ROOT_LIBRARY_DIR)

mark_as_advanced(ROOT_CONFIG_EXECUTABLE)

include(CMakeParseArguments)
find_program(ROOTCLING_EXECUTABLE rootcling
  HINTS "$ENV{ROOTSYS}/bin" "$ENV{BDM_ROOT_DIR}/bin" "${CMAKE_THIRD_PARTY_DIR}/root/bin")
#find_package(GCCXML)

# We use the launcher script to emulate a `source thisbdm.sh` call
if(NOT BDM_OUT_OF_SOURCE)
  set(LAUNCHER ${CMAKE_BINARY_DIR}/launcher.sh)
endif()

#---------------------------------------------------------------------------------------------------
#---ROOT_GENERATE_DICTIONARY( dictionary headerfiles NODEPHEADERS ghdr1 ghdr2 ...
#                                                    MODULE module DEPENDENCIES dep1 dep2
#                                                    BUILTINS dep1 dep2
#                                                    STAGE1 LINKDEF linkdef OPTIONS opt1 opt2 ...)
#
# <dictionary> is the dictionary stem; the macro creates (among other files) the dictionary source as
#   <dictionary>.cxx
# <headerfiles> are "as included"; set appropriate INCLUDE_DIRECTORIES property on the directory.
#   The dictionary target depends on these headers. These files must exist.
# <NODEPHEADERS> same as <headerfiles>. If these files are not found (given the target include path)
#   no error is emitted. The dictionary does not depend on these headers.
# <REFLEX> is for enabling rootcling's reflex mode. In this mode the LinkDef needs to be an XML Selection File
#---------------------------------------------------------------------------------------------------
function(ROOT_GENERATE_DICTIONARY dictionary)
  CMAKE_PARSE_ARGUMENTS(ARG "STAGE1;MULTIDICT;NOINSTALL;NO_CXXMODULE;REFLEX"
    "MODULE;LINKDEF" "NODEPHEADERS;OPTIONS;DEPENDENCIES;EXTRA_DEPENDENCIES;BUILTINS" ${ARGN})

  # Check if OPTIONS start with a dash.
  if (ARG_OPTIONS)
    foreach(ARG_O ${ARG_OPTIONS})
      if (NOT ARG_O MATCHES "^-*")
        message(FATAL_ERROR "Wrong rootcling option: ${ARG_OPTIONS}")
      endif()
    endforeach()
  endif(ARG_OPTIONS)

  #---roottest compability---------------------------------
  if(CMAKE_ROOTTEST_DICT)
    set(CMAKE_INSTALL_LIBDIR ${CMAKE_CURRENT_BINARY_DIR})
    set(libprefix "")
  endif()

  # list of include directories for dictionary generation
  set(incdirs)

  if((CMAKE_PROJECT_NAME STREQUAL ROOT) AND (TARGET ${ARG_MODULE}))
    set(headerdirs)

    get_target_property(target_incdirs ${ARG_MODULE} INCLUDE_DIRECTORIES)
    if(target_incdirs)
       foreach(dir ${target_incdirs})
          ROOT_REPLACE_BUILD_INTERFACE(dir ${dir})
          # check that dir not a empty dir like $<BUILD_INTERFACE:>
          if(NOT ${dir} MATCHES "^[$]")
            list(APPEND incdirs ${dir})
            string(FIND ${dir} "${CMAKE_SOURCE_DIR}" src_dir_in_dir)
            if(${src_dir_in_dir} EQUAL 0)
              list(APPEND headerdirs ${dir})
            endif()
          endif()
       endforeach()
    endif()

    # if (cxxmodules OR runtime_cxxmodules)
    # Comments from Vassil:
    # FIXME: We prepend ROOTSYS/include because if we have built a module
    # and try to resolve the 'same' header from a different location we will
    # get a redefinition error.
    # We should remove these lines when the fallback include is removed. Then
    # we will need a module.modulemap file per `inc` directory.
    # Comments from Sergey:
    # Remove all source dirs also while they preserved in root dictionaries and
    # ends in the gInterpreter->GetIncludePath()

    list(FILTER incdirs EXCLUDE REGEX "^${CMAKE_SOURCE_DIR}")
    list(FILTER incdirs EXCLUDE REGEX "^${CMAKE_BINARY_DIR}/ginclude")
    list(FILTER incdirs EXCLUDE REGEX "^${CMAKE_BINARY_DIR}/externals")
    list(FILTER incdirs EXCLUDE REGEX "^${CMAKE_BINARY_DIR}/builtins")
    list(INSERT incdirs 0 ${CMAKE_BINARY_DIR}/include)
    # endif()

    # this instruct rootcling do not store such paths in dictionary
    set(excludepaths ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR}/ginclude ${CMAKE_BINARY_DIR}/externals ${CMAKE_BINARY_DIR}/builtins)

    set(headerfiles)
    set(_list_of_header_dependencies)
    foreach(fp ${ARG_UNPARSED_ARGUMENTS})
       if(IS_ABSOLUTE ${fp})
          set(headerFile ${fp})
       else()
          find_file(headerFile ${fp}
                  HINTS ${headerdirs}
                  NO_DEFAULT_PATH
                  NO_SYSTEM_ENVIRONMENT_PATH
                  NO_CMAKE_FIND_ROOT_PATH)
       endif()
       if(NOT headerFile)
          message(FATAL_ERROR "Cannot find header ${fp} to generate dictionary ${dictionary} for. Did you forget to set the INCLUDE_DIRECTORIES property for the current directory?")
       endif()
       list(APPEND headerfiles ${fp})
       list(APPEND _list_of_header_dependencies ${headerFile})
       unset(headerFile CACHE) # find_file, forget headerFile!
    endforeach()

    foreach(fp ${ARG_NODEPHEADERS})
      list(APPEND headerfiles ${fp})
      # no dependency - think "vector" etc.
    endforeach()

    if(NOT (headerfiles OR ARG_LINKDEF))
      message(FATAL_ERROR "No headers nor LinkDef.h supplied / found for dictionary ${dictionary}!")
    endif()

  else()

    ####################### old-style includes/headers generation - starts ##################

    #---Get the list of include directories------------------
    get_directory_property(incdirs INCLUDE_DIRECTORIES)
    # rootcling invoked on foo.h should find foo.h in the current source dir,
    # no matter what.
    list(APPEND incdirs ${CMAKE_CURRENT_SOURCE_DIR})

    if(TARGET ${ARG_MODULE})
      get_target_property(target_incdirs ${ARG_MODULE} INCLUDE_DIRECTORIES)
      foreach(dir ${target_incdirs})
        ROOT_REPLACE_BUILD_INTERFACE(dir ${dir})
        if(NOT ${dir} MATCHES "^[$]")
          list(APPEND incdirs ${dir})
        endif()
      endforeach()
    endif()

    set(headerdirs_dflt)

    if(CMAKE_PROJECT_NAME STREQUAL ROOT)
      if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/inc)
        list(APPEND headerdirs_dflt ${CMAKE_CURRENT_SOURCE_DIR}/inc)
      endif()
      if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/v7/inc)
        list(APPEND headerdirs_dflt ${CMAKE_CURRENT_SOURCE_DIR}/v7/inc)
      endif()
    endif()

    #---Get the list of header files-------------------------
    # CMake needs dependencies from ${CMAKE_CURRENT_SOURCE_DIR} while rootcling wants
    # header files "as included" (and thus as passed as argument to this CMake function).
    set(headerfiles)
    set(_list_of_header_dependencies)
    foreach(fp ${ARG_UNPARSED_ARGUMENTS})
      if(${fp} MATCHES "[*?]") # Is this header a globbing expression?
        file(GLOB files inc/${fp} ${fp}) # Elements of ${fp} have the complete path.
        foreach(f ${files})
          if(NOT f MATCHES LinkDef) # skip LinkDefs from globbing result
            set(add_inc_as_include On)
            string(REGEX REPLACE "^${CMAKE_CURRENT_SOURCE_DIR}/inc/" "" f_no_inc ${f})
            list(APPEND headerfiles ${f_no_inc})
            list(APPEND _list_of_header_dependencies ${f})
          endif()
        endforeach()
      else()
        if(IS_ABSOLUTE ${fp})
          set(headerFile ${fp})
        else()
          set(incdirs_in_build)
          set(incdirs_in_prefix ${headerdirs_dflt})
          foreach(incdir ${incdirs})
            string(FIND ${incdir} "${CMAKE_SOURCE_DIR}" src_dir_in_dir)
            string(FIND ${incdir} "${CMAKE_BINARY_DIR}" bin_dir_in_dir)
            string(FIND ${incdir} "${CMAKE_CURRENT_BINARY_DIR}" cur_dir_in_dir)
            if(NOT IS_ABSOLUTE ${incdir}
               OR ${src_dir_in_dir} EQUAL 0
               OR ${bin_dir_in_dir} EQUAL 0
               OR ${cur_dir_in_dir} EQUAL 0)
              list(APPEND incdirs_in_build ${incdir})
            else()
              list(APPEND incdirs_in_prefix ${incdir})
            endif()
          endforeach()
          if(incdirs_in_build)
            find_file(headerFile ${fp}
              HINTS ${incdirs_in_build}
              NO_DEFAULT_PATH
              NO_SYSTEM_ENVIRONMENT_PATH
              NO_CMAKE_FIND_ROOT_PATH)
          endif()
          # Try this even if NOT incdirs_in_prefix: might not need a HINT.
          if(NOT headerFile)
            find_file(headerFile ${fp}
              HINTS ${incdirs_in_prefix}
              NO_DEFAULT_PATH
              NO_SYSTEM_ENVIRONMENT_PATH)
          endif()
        endif()
        if(NOT headerFile)
          message(FATAL_ERROR "Cannot find header ${fp} to generate dictionary ${dictionary} for. Did you forget to set the INCLUDE_DIRECTORIES property for the current directory?")
        endif()
        list(APPEND headerfiles ${fp})
        list(APPEND _list_of_header_dependencies ${headerFile})
        unset(headerFile CACHE) # find_file, forget headerFile!
      endif()
    endforeach()

    foreach(fp ${ARG_NODEPHEADERS})
      list(APPEND headerfiles ${fp})
      # no dependency - think "vector" etc.
    endforeach()

    if(NOT (headerfiles OR ARG_LINKDEF))
      message(FATAL_ERROR "No headers nor LinkDef.h supplied / found for dictionary ${dictionary}!")
    endif()

    if(CMAKE_PROJECT_NAME STREQUAL ROOT)
      list(APPEND incdirs ${CMAKE_BINARY_DIR}/include)
      list(APPEND incdirs ${CMAKE_BINARY_DIR}/etc/cling) # This is for the RuntimeUniverse
      # list(APPEND incdirs ${CMAKE_SOURCE_DIR})
      set(excludepaths ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})
    elseif(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/inc)
      list(APPEND incdirs ${CMAKE_CURRENT_SOURCE_DIR}/inc)
    endif()

    foreach(dep ${ARG_DEPENDENCIES})
      if(TARGET ${dep})
        get_target_property(dep_include_dirs ${dep} INTERFACE_INCLUDE_DIRECTORIES)
        if (NOT dep_include_dirs)
          get_target_property(dep_include_dirs ${dep} INCLUDE_DIRECTORIES)
        endif()
        if (dep_include_dirs)
          foreach(d ${dep_include_dirs})
            list(APPEND incdirs ${d})
          endforeach()
        endif()
      endif()
    endforeach()

    ####################### old-style includes/headers generation - end  ##################
  endif()

  #---Get the list of definitions---------------------------
  get_directory_property(defs COMPILE_DEFINITIONS)
  foreach( d ${defs})
   if((NOT d MATCHES "=") AND (NOT d MATCHES "^[$]<.*>$")) # avoid generator expressions
     set(definitions ${definitions} -D${d})
   endif()
  endforeach()
  #---Get LinkDef.h file------------------------------------
  foreach( f ${ARG_LINKDEF})
    if( IS_ABSOLUTE ${f})
      set(_linkdef ${_linkdef} ${f})
    else()
      if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/inc/${f})
        set(_linkdef ${_linkdef} ${CMAKE_CURRENT_SOURCE_DIR}/inc/${f})
      else()
        set(_linkdef ${_linkdef} ${CMAKE_CURRENT_SOURCE_DIR}/${f})
      endif()
    endif()
  endforeach()

  #---Build the names for library, pcm and rootmap file ----
  set(library_target_name)
  if(dictionary MATCHES "^G__")
    string(REGEX REPLACE "^G__(.*)" "\\1"  library_target_name ${dictionary})
    if (ARG_MULTIDICT)
      string(REGEX REPLACE "(.*)32$" "\\1"  library_target_name ${library_target_name})
    endif (ARG_MULTIDICT)
  else()
    get_filename_component(library_target_name ${dictionary} NAME_WE)
  endif()
  if (ARG_MODULE)
    if (NOT ${ARG_MODULE} STREQUAL ${library_target_name})
#      message(AUTHOR_WARNING "The MODULE argument ${ARG_MODULE} and the deduced library name "
#        "${library_target_name} mismatch. Deduction stem: ${dictionary}.")
      set(library_target_name ${ARG_MODULE})
    endif()
  endif(ARG_MODULE)

  #---Set the library output directory-----------------------
  ROOT_GET_LIBRARY_OUTPUT_DIR(library_output_dir)
  set(runtime_cxxmodule_dependencies )
  set(cpp_module)
  set(library_name ${libprefix}${library_target_name}${libsuffix})
  set(newargs -s ${library_output_dir}/${library_name})
  set(rootmap_name ${library_output_dir}/${libprefix}${library_target_name}.rootmap)
  set(pcm_name ${library_output_dir}/${libprefix}${library_target_name}_rdict.pcm)
  if(ARG_MODULE)
    if(ARG_MULTIDICT)
      set(newargs ${newargs} -multiDict)
      set(pcm_name ${library_output_dir}/${libprefix}${library_target_name}_${dictionary}_rdict.pcm)
      set(rootmap_name ${library_output_dir}/${libprefix}${library_target_name}32.rootmap)
    else()
      set(cpp_module ${library_target_name})
    endif(ARG_MULTIDICT)

    if(runtime_cxxmodules)
      # If we specify NO_CXXMODULE we should be able to still install the produced _rdict.pcm file.
      if(NOT ARG_NO_CXXMODULE)
        set(pcm_name)
      endif()
      if(cpp_module)
        set(cpp_module_file ${library_output_dir}/${cpp_module}.pcm)
        # The module depends on its modulemap file.
        if (cpp_module_file AND CMAKE_PROJECT_NAME STREQUAL ROOT)
		set (runtime_cxxmodule_dependencies copymodulemap "${CMAKE_BINARY_DIR}/include/ROOT.modulemap")
        endif()
      endif(cpp_module)
    endif()
  endif()

  # modules.idx deps
  get_property(local_modules_idx_deps GLOBAL PROPERTY modules_idx_deps_property)
  get_property(local_no_cxxmodules GLOBAL PROPERTY no_cxxmodules_property)
  if (ARG_NO_CXXMODULE)
    list(APPEND local_no_cxxmodules ${cpp_module})
    set_property(GLOBAL PROPERTY no_cxxmodules_property "${local_no_cxxmodules}")
    unset(cpp_module)
    unset(cpp_module_file)
  else()
    list(APPEND local_modules_idx_deps ${cpp_module})
    set_property(GLOBAL PROPERTY modules_idx_deps_property "${local_modules_idx_deps}")
  endif(ARG_NO_CXXMODULE)



  if(CMAKE_ROOTTEST_NOROOTMAP OR cpp_module_file)
    set(rootmap_name)
    set(rootmapargs)
  else()
    set(rootmapargs -rml ${library_name} -rmf ${rootmap_name})
  endif()

  #---Get the library and module dependencies-----------------
  if(ARG_DEPENDENCIES)
    foreach(dep ${ARG_DEPENDENCIES})
      set(dependent_pcm ${libprefix}${dep}_rdict.pcm)
      if (runtime_cxxmodules AND NOT dep IN_LIST local_no_cxxmodules)
        set(dependent_pcm ${dep}.pcm)
      endif()
      set(newargs ${newargs} -m  ${dependent_pcm})
    endforeach()
  endif()

  if(cpp_module_file)
    set(newargs -cxxmodule ${newargs})
  endif()
  
  if(ARG_REFLEX)
    set(newargs -reflex ${newargs})
  endif()

  #---what rootcling command to use--------------------------
  if(ARG_STAGE1)
    if(MSVC AND CMAKE_ROOTTEST_DICT)
      set(command ${CMAKE_COMMAND} -E ${CMAKE_BINARY_DIR}/bin/rootcling_stage1.exe)
    else()
      set(command ${CMAKE_COMMAND} -E env "LD_LIBRARY_PATH=${CMAKE_BINARY_DIR}/lib:$ENV{LD_LIBRARY_PATH}" $<TARGET_FILE:rootcling_stage1>)
    endif()
    set(ROOTCINTDEP rconfigure)
    set(pcm_name)
  else()
    if(CMAKE_PROJECT_NAME STREQUAL ROOT)
      if(MSVC AND CMAKE_ROOTTEST_DICT)
        set(command ${CMAKE_COMMAND} -E env "ROOTIGNOREPREFIX=1" ${CMAKE_BINARY_DIR}/bin/rootcling.exe)
      else()
        set(command ${CMAKE_COMMAND} -E env "LD_LIBRARY_PATH=${CMAKE_BINARY_DIR}/lib:$ENV{LD_LIBRARY_PATH}"
                    "ROOTIGNOREPREFIX=1" $<TARGET_FILE:rootcling> -rootbuild)
        # Modules need RConfigure.h copied into include/.
        set(ROOTCINTDEP rootcling rconfigure)
      endif()
    elseif(TARGET ROOT::rootcling)
      if(APPLE)
        set(command ${CMAKE_COMMAND} -E env "DYLD_LIBRARY_PATH=${ROOT_LIBRARY_DIR}:$ENV{DYLD_LIBRARY_PATH}" $<TARGET_FILE:ROOT::rootcling>)
      else()
        set(command ${CMAKE_COMMAND} -E env "LD_LIBRARY_PATH=${ROOT_LIBRARY_DIR}:$ENV{LD_LIBRARY_PATH}" $<TARGET_FILE:ROOT::rootcling>)
      endif()
    else()
      set(command ${CMAKE_COMMAND} -E env rootcling)
    endif()
  endif()

  #---build the path exclusion switches----------------------
  set(excludepathsargs "")
  foreach(excludepath ${excludepaths})
    set(excludepathsargs ${excludepathsargs} -excludePath ${excludepath})
  endforeach()

  #---build the implicit dependencies arguments
  # NOTE: only the Makefile generator respects this!
  foreach(_dep ${_linkdef} ${_list_of_header_dependencies})
    list(APPEND _implicitdeps CXX ${_dep})
  endforeach()

  if(ARG_MODULE)
    set(MODULE_LIB_DEPENDENCY ${ARG_DEPENDENCIES})

    # get target properties added after call to ROOT_GENERATE_DICTIONARY()
    if(TARGET ${ARG_MODULE})
      # NOTE that module_sysincs is already part of ${module_sysincs}. But -isystem "wins",
      # and list exclusion for generator expressions is too complex.
      set(module_incs $<REMOVE_DUPLICATES:$<TARGET_PROPERTY:${ARG_MODULE},INCLUDE_DIRECTORIES>>)
      set(module_sysincs $<REMOVE_DUPLICATES:$<TARGET_PROPERTY:${ARG_MODULE},INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>>)
      # The COMPILE_DEFINITIONS list might contain empty elements. These are
      # removed with the FILTER generator expression, excluding elements that
      # match the ^$ regexp (only matches empty strings).
      set(module_defs "$<FILTER:$<TARGET_PROPERTY:${ARG_MODULE},COMPILE_DEFINITIONS>,EXCLUDE,^$>")
    endif()
  endif()

  # provide list of includes for dictionary
  set(includedirs)
  if(incdirs)
     list(REMOVE_DUPLICATES incdirs)
     foreach(dir ${incdirs})
        if (NOT ${dir} MATCHES "^\\$<INSTALL_INTERFACE:")
          list(APPEND includedirs -I${dir})
        endif()
     endforeach()
  endif()

  set(compIncPaths)
  foreach(implinc IN LISTS CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES)
    list(APPEND compIncPaths "-compilerI${implinc}")
  endforeach()

  #---call rootcint------------------------------------------
  add_custom_command(OUTPUT ${dictionary}.cxx ${pcm_name} ${rootmap_name} ${cpp_module_file}
                     COMMAND ${command} -v2 -f  ${dictionary}.cxx ${newargs} ${excludepathsargs} ${rootmapargs}
                                        ${ARG_OPTIONS}
                                        ${definitions} "$<$<BOOL:${module_defs}>:-D$<JOIN:${module_defs},;-D>>"
                                        ${compIncPaths}
                                        "$<$<BOOL:${module_sysincs}>:-isystem;$<JOIN:${module_sysincs},;-isystem;>>"
                                        ${includedirs} "$<$<BOOL:${module_incs}>:-I$<JOIN:${module_incs},;-I>>"
                                        ${headerfiles} ${_linkdef}
                     IMPLICIT_DEPENDS ${_implicitdeps}
                     DEPENDS ${_list_of_header_dependencies} ${_linkdef} ${ROOTCINTDEP}
                             ${MODULE_LIB_DEPENDENCY} ${ARG_EXTRA_DEPENDENCIES}
                             ${runtime_cxxmodule_dependencies}
                     COMMAND_EXPAND_LISTS)

  # If we are adding to an existing target and it's not the dictionary itself,
  # we make an object library and add its output object file as source to the target.
  # This works around bug https://cmake.org/Bug/view.php?id=14633 in CMake by keeping
  # the generated source at the same scope level as its owning target, something that
  # would not happen if we used target_sources() directly with the dictionary source.
  if(TARGET "${ARG_MODULE}" AND NOT "${ARG_MODULE}" STREQUAL "${dictionary}")
    add_library(${dictionary} OBJECT ${dictionary}.cxx)
    set_target_properties(${dictionary} PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
    target_sources(${ARG_MODULE} PRIVATE $<TARGET_OBJECTS:${dictionary}>)

    target_compile_options(${dictionary} PRIVATE
      $<TARGET_PROPERTY:${ARG_MODULE},COMPILE_OPTIONS>)

    target_compile_definitions(${dictionary} PRIVATE
      ${definitions} $<TARGET_PROPERTY:${ARG_MODULE},COMPILE_DEFINITIONS>)

    target_compile_features(${dictionary} PRIVATE
      $<TARGET_PROPERTY:${ARG_MODULE},COMPILE_FEATURES>)

    target_include_directories(${dictionary} PRIVATE
      ${incdirs} $<TARGET_PROPERTY:${ARG_MODULE},INCLUDE_DIRECTORIES>)
  else()
    get_filename_component(dictionary_name ${dictionary} NAME)
    add_custom_target(${dictionary_name} DEPENDS ${dictionary}.cxx ${pcm_name} ${rootmap_name} ${cpp_module_file})
  endif()

  if(PROJECT_NAME STREQUAL "ROOT")
    set_property(GLOBAL APPEND PROPERTY ROOT_PCH_DEPENDENCIES ${dictionary})
    set_property(GLOBAL APPEND PROPERTY ROOT_PCH_DICTIONARIES ${CMAKE_CURRENT_BINARY_DIR}/${dictionary}.cxx)
  endif()

  if(ARG_MULTIDICT)
    if(NOT TARGET "G__${ARG_MODULE}")
      message(FATAL_ERROR
        " Target G__${ARG_MODULE} not found!\n"
        " Please create target G__${ARG_MODULE} before using MULTIDICT.")
    endif()
    add_dependencies(G__${ARG_MODULE} ${dictionary})
  endif()

  if(NOT ARG_NOINSTALL AND NOT CMAKE_ROOTTEST_DICT AND DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    ROOT_GET_INSTALL_DIR(shared_lib_install_dir)
    # Install the C++ module if we generated one.
    if (cpp_module_file)
      install(FILES ${cpp_module_file}
                    DESTINATION ${shared_lib_install_dir} COMPONENT libraries)
    endif()

    if(ARG_STAGE1)
      install(FILES ${rootmap_name}
                    DESTINATION ${shared_lib_install_dir} COMPONENT libraries)
    else()
      install(FILES ${pcm_name} ${rootmap_name}
                    DESTINATION ${shared_lib_install_dir} COMPONENT libraries)
    endif()
  endif()

  if(ARG_BUILTINS)
    foreach(arg1 ${ARG_BUILTINS})
      if(TARGET ${${arg1}_TARGET})
        add_dependencies(${dictionary} ${${arg1}_TARGET})
      endif()
    endforeach()
  endif()

  # FIXME: Support mulptiple dictionaries. In some cases (libSMatrix and
  # libGenVector) we have to have two or more dictionaries (eg. for math,
  # we need the two for double vs Double32_t template specializations).
  # In some other cases, eg. libTreePlayer.so we add in a separate dictionary
  # files which for some reason (temporarily?) cannot be put in the PCH. Eg.
  # all rest of the first dict is in the PCH but this file is not and it
  # cannot be present in the original dictionary.
  if(cpp_module)
    ROOT_CXXMODULES_APPEND_TO_MODULEMAP("${cpp_module}" "${headerfiles}")
  endif()
endfunction(ROOT_GENERATE_DICTIONARY)

#----------------------------------------------------------------------------
# function REFLEX_GENERATE_DICTIONARY(dictionary
#                                     header1 header2 ...
#                                     SELECTION selectionfile ...
#                                     OPTIONS opt1...)
function(REFLEX_GENERATE_DICTIONARY dictionary)
  CMAKE_PARSE_ARGUMENTS(ARG "" "" "SELECTION;OPTIONS" "" ${ARGN})
  #---Get the list of header files-------------------------
  set(headerfiles)
  foreach(fp ${ARG_UNPARSED_ARGUMENTS})
    file(GLOB files ${fp})
    if(files)
      foreach(f ${files})
        set(headerfiles ${headerfiles} ${f})
      endforeach()
    else()
      set(headerfiles ${headerfiles} ${fp})
    endif()
  endforeach()
  #---Get Selection file------------------------------------
  if(IS_ABSOLUTE ${ARG_SELECTION})
    set(selectionfile ${ARG_SELECTION})
  else()
    set(selectionfile ${CMAKE_CURRENT_SOURCE_DIR}/${ARG_SELECTION})
  endif()
  #---Get the list of include directories------------------
  get_directory_property(incdirs INCLUDE_DIRECTORIES)
  set(includedirs)
  foreach( d ${incdirs})
    set(includedirs ${includedirs} -I${d})
  endforeach()
  #---Get preprocessor definitions--------------------------
  get_directory_property(defs COMPILE_DEFINITIONS)
  foreach( d ${defs})
   set(definitions ${definitions} -D${d})
  endforeach()
  #---Nanes and others---------------------------------------
  set(gensrcdict ${dictionary}.cc)
  if(MSVC)
    set(gccxmlopts "--gccxmlopt=\"--gccxml-compiler cl\"")
  else()
    #set(gccxmlopts "--gccxmlopt=\'--gccxml-cxxflags -m64 \'")
    set(gccxmlopts)
  endif()
  #set(rootmapname ${dictionary}Dict.rootmap)
  #set(rootmapopts --rootmap=${rootmapname} --rootmap-lib=${libprefix}${dictionary}Dict)
  #---Actual command----------------------------------------
  add_custom_command(OUTPUT ${gensrcdict} ${rootmapname} ${dictionary}.pcm
    COMMAND ${LAUNCHER} ${ROOTCLING_EXECUTABLE} -reflex -cxxmodule -o ${gensrcdict} ${rootmapopts} ${headerfiles} ${selectionfile} -noIncludePaths -inlineInputHeader
                            ${ARG_OPTIONS} ${includedirs} ${definitions}
                     DEPENDS ${headerfiles} ${selectionfile})
endfunction()
