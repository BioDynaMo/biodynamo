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

#####################################
## Building ParaView for BioDynaMo ##
#####################################

if [[ $# -ne 1 ]]; then
  echo "ERROR: Wrong number of arguments.
Description:
  This script builds paraview.
  The archive will be stored in BDM_PROJECT_DIR/build/paraview.tar.gz
Arguments:
  \$1 Paraview version that should be build"
  exit 1
fi

set -e -x

PV_VERSION=$1
shift
BDM_PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../.."
cd $BDM_PROJECT_DIR

# import util functions
. $BDM_PROJECT_DIR/util/installation/common/util.sh

# archive destination dir
DEST_DIR=$BDM_PROJECT_DIR/build
mkdir -p $DEST_DIR
EchoNewStep "Start building ParaView $PV_VERSION. Result will be stored in $DEST_DIR"
# working dir
WORKING_DIR=~/bdm-build-third-party
mkdir -p $WORKING_DIR
cd $WORKING_DIR

# Install Qt (prerequisites will be installed inside build-qt.sh)
. $BDM_PROJECT_DIR/util/build-third-party/build-qt.sh
cd $WORKING_DIR

if [ `uname` = "Linux" ]; then
  export QT_CMAKE_DIR=$WORKING_DIR/qt/5.11.0/gcc_64/lib/cmake
  export LD_LIBRARY_PATH=$WORKING_DIR/qt/5.11.0/gcc_64/lib:$LD_LIBRARY_PATH
else
  export QT_CMAKE_DIR=$WORKING_DIR/qt/5.11.0/clang_64/lib/cmake
fi

## Clone paraview github repository
git clone https://gitlab.kitware.com/paraview/paraview-superbuild.git
cd paraview-superbuild
git fetch origin
git submodule update --init --recursive
git checkout $PV_VERSION
git submodule update --init --recursive

## Generate the cmake files for paraview
#
# If you want to build against a specific Qt library at /path/to/qt/cmake
# then set CMAKE_PREFIX_PATH=/path/to/qt/cmake (in this dir all the Qt modules
# should be available)

mkdir -p ../paraview-build
cd ../paraview-build

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
cmake \
  -DCMAKE_INSTALL_PREFIX="../pv-install" \
  -DCMAKE_BUILD_TYPE:STRING="Release" \
  -DENABLE_ospray:BOOL=ON \
  -DENABLE_ospraymaterials:BOOL=ON \
  -DENABLE_paraviewsdk:BOOL=ON \
  -DENABLE_python:BOOL=ON \
  -DENABLE_python3:BOOL=ON \
  -DENABLE_qt5:BOOL=ON \
  -DUSE_SYSTEM_qt5:BOOL=ON \
  -DENABLE_mpi:BOOL=ON \
  -DUSE_SYSTEM_mpi:BOOL=ON \
  ../paraview-superbuild

## Step 4: compile and install
make -j2 install

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

cd install

# For some reason this path is hardcoded in this file, which causes CMake to
# panic. We just remove it.
sed -i 's|/home/testuser/bdm-build-third-party/paraview-build/install/include/python3.7m||g' lib/cmake/paraview-5.8/vtk/VTK-targets.cmake || true


## tar the install directory
tar -zcf paraview-$PV_VERSION.tar.gz *

# After untarring the directory tree should like like this:
# paraview
#   |-- bin
#   |-- include
#   |-- lib
#   |-- share

# Step 5: cp to destination directory
cp paraview-$PV_VERSION.tar.gz $DEST_DIR
