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

# OpenMP
export OMP_PROC_BIND=true

unset old_bdmsys
unset old_bdmsym_base
unset thisbdm

echo "You have successfully sourced BioDynaMo's environment."
