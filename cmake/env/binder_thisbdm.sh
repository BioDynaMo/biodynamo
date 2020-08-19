#!/usr/bin/env bash
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
# This script is sourced prior to launching a Binder notebook to find BioDynaMo
#
# Author: Fons Rademakers, 16/5/2019

if [ -n "${BDM_INSTALL_DIR}" ] ; then
   old_bdmsys_base=${BDM_INSTALL_DIR}
fi
if [ -n "${BDMSYS}" ] ; then
   old_bdmsys=${BDMSYS}
fi

export BDM_INSTALL_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
export BDMSYS="${BDM_INSTALL_DIR}"

if [ -z "${MANPATH}" ]; then
   # Grab the default man path before setting the path to avoid duplicates
   if command -v manpath >/dev/null; then
      default_manpath=`manpath`
   elif command -v man >/dev/null; then
      default_manpath=`man -w 2> /dev/null`
   else
      default_manpath=""
   fi
fi

if [ -z "${PATH}" ]; then
   PATH="${BDMSYS}/bin"; export PATH
else
   PATH="${BDMSYS}/bin":$PATH; export PATH
fi

if [ -z "${LD_LIBRARY_PATH}" ]; then
   LD_LIBRARY_PATH="${BDMSYS}/lib"; export LD_LIBRARY_PATH       # Linux, ELF HP-UX
else
   LD_LIBRARY_PATH="${BDMSYS}/lib":$LD_LIBRARY_PATH; export LD_LIBRARY_PATH
fi

if [ -z "${DYLD_LIBRARY_PATH}" ]; then
   DYLD_LIBRARY_PATH="${BDMSYS}/lib"; export DYLD_LIBRARY_PATH   # Mac OS X
else
   DYLD_LIBRARY_PATH="${BDMSYS}/lib":$DYLD_LIBRARY_PATH; export DYLD_LIBRARY_PATH
fi

if [ -z "${SHLIB_PATH}" ]; then
   SHLIB_PATH="${BDMSYS}/lib"; export SHLIB_PATH                 # legacy HP-UX
else
   SHLIB_PATH="${BDMSYS}/lib":$SHLIB_PATH; export SHLIB_PATH
fi

if [ -z "${LIBPATH}" ]; then
   LIBPATH="${BDMSYS}/lib"; export LIBPATH                       # AIX
else
   LIBPATH="${BDMSYS}/lib":$LIBPATH; export LIBPATH
fi

if [ -z "${MANPATH}" ]; then
   MANPATH="${BDMSYS}/man":${default_manpath}; export MANPATH
else
   MANPATH="${BDMSYS}/man":$MANPATH; export MANPATH
fi

##### CMake Specific Configurations #####
if [ -z "${CMAKE_PREFIX_PATH}" ]; then
   CMAKE_PREFIX_PATH="${BDMSYS}/share/cmake"; export CMAKE_PREFIX_PATH       # Linux, ELF HP-UX
else
   CMAKE_PREFIX_PATH="${BDMSYS}/share/cmake":$CMAKE_PREFIX_PATH; export CMAKE_PREFIX_PATH
fi

BDM_CMAKE_DIR="${BDMSYS}/share/cmake"; export BDM_CMAKE_DIR
BDM_SRC_DIR="${BDMSYS}/include"; export BDM_SRC_DIR
########

#### ROOT Specific Configurations ####
if [ -z ${BDM_ROOT_DIR} ] && [ -z ${ROOTSYS} ]; then
    BDM_ROOT_DIR=${BDMSYS}/third_party/root
    if ! [ -d $BDM_ROOT_DIR ]; then
        echo "We are unable to source ROOT! Please make sure ROOT is installed on your system!"
        echo "You can specify manually its location by executing 'export BDM_ROOT_DIR=path/to/root'"
        echo "before running cmake."
        echo "Sourcing BioDynaMo env failed!"
        return 1
    fi
else
  # ROOTSYS has precedence over the BDM_ROOT_DIR custom configuration
  if [ -n ${ROOTSYS} ]; then
     orvers="@rootvers@"
     crvers="$($ROOTSYS/bin/root-config --version)"
     if [ $crvers == $orvers ]; then
        BDM_ROOT_DIR=${ROOTSYS}
     else
        echo "ROOTSYS points to ROOT version $crvers, while BDM was build with version $orvers."
        echo "Make sure that ROOTSYS points to the right version of ROOT."
        echo "Sourcing BioDynaMo env failed!"
        return 1
     fi
  fi
fi

. ${BDM_ROOT_DIR}/bin/thisroot.sh

########

# Load the rootlogon.C
unset -f root || true
function root {
  ${BDM_ROOT_DIR}/bin/root -l -e "cout << \"Loading BioDynaMo into ROOT...\" << endl;gROOT->LoadMacro(\"${BDMSYS}/etc/rootlogon.C\");" $@
}

# OpenMP
export OMP_PROC_BIND=true

unset old_bdmsys
unset old_bdmsym_base
unset thisbdm

echo "You have successfully sourced BioDynaMo's environment."
