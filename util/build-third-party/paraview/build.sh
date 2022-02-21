#!/bin/bash
# -----------------------------------------------------------------------------
#
# Copyright (C) 2021 CERN & University of Surrey for the benefit of the
# BioDynaMo collaboration. All Rights Reserved.
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
if [ "$(uname)" = "Linux" ]; then
  export Qt5_DIR=$WORKING_DIR/qt
  export QT_CMAKE_DIR=$WORKING_DIR/qt/lib/cmake/Qt5
  export LD_LIBRARY_PATH=$WORKING_DIR/qt/lib:$LD_LIBRARY_PATH
  CC=gcc
  CXX=g++
  export PATH=$WORKING_DIR/cmake-3.19.3/bin:$PATH
  if [ "$BDM_OS" = "centos-7" ]; then
    set +e
    . /etc/profile.d/modules.sh
    module load mpi
    . scl_source enable devtoolset-8
    set -e
  fi
  export PYENV_ROOT="$HOME/.pyenv"
  export PATH="$PYENV_ROOT/bin:$PATH"
  eval "$(pyenv init --path)"
  eval "$(pyenv init -)"
  pyenv shell 3.9.1
else
  # XCode compilers work fine now
  export CC=clang
  export CXX=clang++
  # From ParaView 5.10 on, we use brew qt@5.
  if [ "$PV_VERSION" = "v5.10.0" ]; then
    export Qt5_DIR=$(brew --prefix)/opt/qt@5
    export QT_CMAKE_DIR=$(brew --prefix)/opt/qt@5/lib/cmake/Qt5
    export DYLD_LIBRARY_PATH=$(brew --prefix)/opt/qt@5/lib:$DYLD_LIBRARY_PATH
  else
    export Qt5_DIR=$WORKING_DIR/qt
    export QT_CMAKE_DIR=$WORKING_DIR/qt/lib/cmake/Qt5
    export DYLD_LIBRARY_PATH=$WORKING_DIR/qt/lib:$DYLD_LIBRARY_PATH
  fi
fi

# The CMAKE_INSTALL_RPATH will put all the specified paths in all the installed
# targets (libraries and binaries) (upon make install). Since the relative paths
# from the ParaView targets are always the same we can set the rpaths to be
# relative from the ParaView targets (which are located at @loader_path). This
# makes the ParaView installation portable (as long as we copy Qt with it)
# -DPARAVIEW_DO_UNIX_STYLE_INSTALLS forces CMake to install OSX build similarly
# to Linux, and enforces the RPATH (instead of @executable_path/../).
# The three RPATHS are respectively as follows:
# 1. ParaView binaries  -> Qt libraries
# 2. ParaView libraries -> Qt libraries
# 3. ParaView binaries / libraries -> ParaView libraries

BDM_PV_BUILD_CMAKE_ARGS="-GNinja
  -DCMAKE_BUILD_TYPE:STRING=Release
  -Dparaview_SOURCE_SELECTION=source
  -Dparaview_SOURCE_DIR=${WORKING_DIR}/src
  -DENABLE_ospray:BOOL=ON
  -DENABLE_ospraymaterials:BOOL=ON
  -DENABLE_paraviewsdk:BOOL=ON
  -DENABLE_python3:BOOL=ON
  -DENABLE_qt5:BOOL=ON
  -DUSE_SYSTEM_qt5:BOOL=ON
  -DENABLE_mpi:BOOL=ON
  -DUSE_SYSTEM_mpi:BOOL=ON
  -DUSE_SYSTEM_python3:BOOL=ON"

if [ "$(uname)" = "Darwin" ]; then
  BDM_MACOS_PY="$(brew --prefix)/bin/python3"
  BDM_PV_BUILD_CMAKE_ARGS="$BDM_PV_BUILD_CMAKE_ARGS
  -DPYTHON_EXECUTABLE=$BDM_MACOS_PY
  -DPARAVIEW_DO_UNIX_STYLE_INSTALLS:BOOL=ON
  -DCMAKE_MACOSX_RPATH:BOOL=ON
  -DCMAKE_INSTALL_RPATH:STRING=@loader_path/../../qt/lib;@loader_path/../../../../../qt/lib;@loader_path/../lib"
  if [ "$PV_VERSION" = "v5.10.0" ]; then
    # For ParaView-5.10 we need to specify the Qt version used for VTK files. 
    # If we do not fix this to 5, the build will fail in the configuration phase
    # because it is incompatible with other options. Possibly, consider moving 
    # to Qt6 entirely once it is supported.
    BDM_PV_BUILD_CMAKE_ARGS="$BDM_PV_BUILD_CMAKE_ARGS
    -DPARAVIEW_EXTRA_CMAKE_ARGUMENTS='-DVTK_QT_VERSION=5'"
  fi
fi

if [ "$PV_FLAVOR" = "default" ]; then
  cmake $(echo $BDM_PV_BUILD_CMAKE_ARGS) ../superbuild
elif [ "$PV_FLAVOR" = "nvidia-headless" ] && ! [ "$(uname)" = "Darwin" ]; then
  cmake \
    -GNinja \
    -DCMAKE_BUILD_TYPE:STRING="RelWithDebInfo" \
    -Dparaview_SOURCE_SELECTION="source" \
    -Dparaview_SOURCE_DIR="${WORKING_DIR}/src" \
    -DENABLE_ospray:BOOL=OFF \
    -DENABLE_ospraymaterials:BOOL=OFF \
    -DENABLE_tbb:BOOL=OFF \
    -DENABLE_paraviewsdk:BOOL=ON \
    -DENABLE_python3:BOOL=ON \
    -DENABLE_egl:BOOL=ON  \
    -DUSE_SYSTEM_egl:BOOL=ON \
    -DENABLE_vtkm:BOOL=ON \
    -DENABLE_mpi:BOOL=ON \
    -DUSE_SYSTEM_mpi:BOOL=ON \
    -DUSE_SYSTEM_python3:BOOL=ON \
    ../superbuild
else
  echo "ERROR: Illegal OS+Flavor combination"
  exit 3
fi

# Workaround; without removing the following file, the paraview build won't
# be started, even if files have changed
rm superbuild/paraview/stamp/paraview-build || true

# compile and install
command -v ninja-build && ninja-build || ninja

# patch and bundle
if ! [ "$(uname)" = "Darwin" ]; then
  cd install
  # For some reason this path is hardcoded in this file, which causes CMake to
  # panic. We just remove it.
  sed -i "s|${PYENV_ROOT}|\$ENV{USER}/.pyenv|g" lib/cmake/paraview-5.9/vtk/VTK-targets.cmake || true
  # Some dependencies could be put into lib64 (e.g. OpenImageDenoise), so we copy
  # it into the lib directory (don't delete lib64, because some CMake files will
  # be referring to that directory)
  # rsync -a lib64/ lib/ || true
fi
