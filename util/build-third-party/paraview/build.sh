#!/bin/bash
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

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
. "${SCRIPT_DIR}/env.sh"

set -e -x

mkdir -p "$SCRIPT_DIR/build"
cd "$SCRIPT_DIR/build"  

# set build environement variables
if [ `uname` = "Linux" ]; then
  export QT_CMAKE_DIR=$WORKING_DIR/qt/lib/cmake/Qt5
  export LD_LIBRARY_PATH=$WORKING_DIR/qt/lib:$LD_LIBRARY_PATH
  CC=gcc
  CXX=g++
  export PATH=$WORKING_DIR/cmake-3.17.0/bin:$PATH
  if [ $BDM_OS = "centos-7" ]; then
    set +e
    . /etc/profile.d/modules.sh
    module load mpi
    . scl_source enable devtoolset-7
    set -e
  elif [ $BDM_OS = "travis-linux" ]; then
      CC=gcc-5
      CXX=g++-5
  fi
else
  export QT_CMAKE_DIR=$WORKING_DIR/qt/5.11.0/clang_64/lib/cmake
  export CXX=/usr/local/opt/llvm@6/bin/clang++
  export CC=/usr/local/opt/llvm@6/bin/clang
  export LLVM_CONFIG="/usr/bin/llvm-config-6"
fi
export PATH="$HOME/.pyenv/bin:$PATH"
export PYENV_ROOT="$HOME/.pyenv"
eval "$(pyenv init -)"
pyenv shell 3.8.0


# The CMAKE_INSTALL_RPATH will put all the specified paths in all the installed
# targets (libraries and binaries) (upon make install). Since the relative paths
# from the ParaView targets are always the same we can set the rpaths to be
# relative from the ParaView targets (which are located at @loader_path). This
# makes the ParaView installation portable (as long as we copy Qt with it)
# DPARAVIEW_DO_UNIX_STYLE_INSTALLS forces CMake to install OSX build similarly
# to Linux, and enforces the RPATH (instead of @executable_path/../).
# The three RPATHS are respectively as follows:
# 1. ParaView binaries  -> Qt libraries
# 2. ParaView libraries -> Qt libraries
# 3. ParaView binaries / libraries -> ParaView libraries

if [ `uname` = "Darwin" ]; then
  OSX_CMAKE_OPTIONS='-DPARAVIEW_DO_UNIX_STYLE_INSTALLS:BOOL=ON
                     -DCMAKE_MACOSX_RPATH:BOOL=ON
                     -DCMAKE_INSTALL_RPATH:STRING=@loader_path/../../qt/lib;@loader_path/../../../../../qt/lib;@loader_path/../lib'
fi

export Qt5_DIR=$QT_CMAKE_DIR
if [ "$PV_FLAVOR" = "default" ]; then
  cmake \
    -GNinja \
    -DCMAKE_BUILD_TYPE:STRING="Release" \
    -Dparaview_SOURCE_SELECTION="source" \
    -Dparaview_SOURCE_DIR="${WORKING_DIR}/src" \
    -DENABLE_ospray:BOOL=ON \
    -DENABLE_ospraymaterials:BOOL=ON \
    -DENABLE_paraviewsdk:BOOL=ON \
    -DENABLE_python:BOOL=ON \
    -DENABLE_python3:BOOL=ON \
    -DENABLE_qt5:BOOL=ON \
    -DUSE_SYSTEM_qt5:BOOL=ON \
    -DENABLE_mpi:BOOL=ON \
    -DUSE_SYSTEM_mpi:BOOL=ON \
    -DUSE_SYSTEM_python3:BOOL=ON \
    ../superbuild
elif [ "$PV_FLAVOR" = "nvidia-headless" ]; then
  cmake \
    -GNinja \
    -DCMAKE_BUILD_TYPE:STRING="RelWithDebInfo" \
    -Dparaview_SOURCE_SELECTION="source" \
    -Dparaview_SOURCE_DIR="${WORKING_DIR}/src" \
    -DENABLE_ospray:BOOL=OFF \
    -DENABLE_ospraymaterials:BOOL=OFF \
    -DENABLE_tbb:BOOL=OFF \
    -DENABLE_paraviewsdk:BOOL=ON \
    -DENABLE_python:BOOL=ON \
    -DENABLE_python3:BOOL=ON \
    -DENABLE_egl:BOOL=ON  \
    -DUSE_SYSTEM_egl:BOOL=ON \
    -DENABLE_vtkm:BOOL=ON \
    -DENABLE_mpi:BOOL=ON \
    -DUSE_SYSTEM_mpi:BOOL=ON \
    -DUSE_SYSTEM_python3:BOOL=ON \
    ../superbuild
fi 

# Workaround; without removing the following file, the paraview build won't
# be started, even if files have changed
rm superbuild/paraview/stamp/paraview-build || true

# compile and install
command -v ninja-build && ninja-build || ninja

cd install

# patch and bundle
# TODO(ahmad): Patch is probably not necessary anymore after relying on rpath
# on OS X. To be investigated.
if [ `uname` = "Darwin" ]; then
  ## Patch vtkkwProcessXML-pv5.5
  # make install does not set the rpath correctly on OSX
  install_name_tool -add_rpath "@loader_path/../../qt/lib" bin/vtkkwProcessXML-pv5.5
  install_name_tool -add_rpath "@loader_path/../../../../../qt/lib" bin/vtkkwProcessXML-pv5.5
  install_name_tool -add_rpath "@loader_path/../lib" bin/vtkkwProcessXML-pv5.5
fi

# For some reason this path is hardcoded in this file, which causes CMake to
# panic. We just remove it.
# TODO(lukas) use different path on MacOS
sed -i "s|${PYENV_ROOT}|\$ENV{USER}/.pyenv|g" lib/cmake/paraview-5.8/vtk/VTK-targets.cmake || true

# Some dependencies could be put into lib64 (e.g. OpenImageDenoise), so we copy
# it into the lib directory (don't delete lib64, because some CMake files will
# be referring to that directory)
# rsync -a lib64/ lib/ || true

