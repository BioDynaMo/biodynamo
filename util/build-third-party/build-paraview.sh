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

# Install prerequisites
. $BDM_PROJECT_DIR/util/build-third-party/third-party-prerequisites.sh

## Install Qt using the silent JavaScript installer
$BDM_PROJECT_DIR/util/build-third-party/build-qt.sh

if [ `uname` = "Linux" ]; then
  QT_CMAKE_DIR=$WORKING_DIR/qt/5.11.0/gcc_64/lib/cmake
else
  QT_CMAKE_DIR=$WORKING_DIR/qt/5.11.0/clang_64/lib/cmake
fi

## Clone paraview github repository
git clone https://gitlab.kitware.com/paraview/paraview.git
cd paraview
git submodule update --init --recursive
git checkout $PV_VERSION
git submodule update --init --recursive
if [ $PV_VERSION = "v5.5.1" ]; then
  # fix qt 5.11 compilation issue:
  # https://gitlab.kitware.com/paraview/paraview/merge_requests/2474
  git cherry-pick 931c779d
fi

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

Qt5_DIR=$QT_CMAKE_DIR cmake \
  -DPARAVIEW_DO_UNIX_STYLE_INSTALLS:BOOL=ON \
  -DCMAKE_MACOSX_RPATH:BOOL=ON \
  -DCMAKE_INSTALL_PREFIX="../paraview-install" \
  -DCMAKE_BUILD_TYPE:STRING="Release" \
  -DPARAVIEW_ENABLE_PYTHON:BOOL=ON \
  -DPARAVIEW_ENABLE_MPI:BOOL=OFF \
  -DPARAVIEW_INSTALL_DEVELOPMENT_FILES:BOOL=ON \
  -DCMAKE_INSTALL_RPATH:STRING="@loader_path/../../qt/lib;@loader_path/../../../../../qt/lib;@loader_path/../lib" \
   ../paraview

## Step 4: compile and install
make -j$(CPUCount)
make install -j$(CPUCount)

if [ `uname` = "Darwin" ]; then
  ## Patch vtkkwProcessXML-pv5.5
  # make install does not set the rpath correctly on OSX
  install_name_tool -add_rpath "@loader_path/../../qt/lib" bin/vtkkwProcessXML-pv5.5
  install_name_tool -add_rpath "@loader_path/../../../../../qt/lib" bin/vtkkwProcessXML-pv5.5
  install_name_tool -add_rpath "@loader_path/../lib" bin/vtkkwProcessXML-pv5.5
fi

## tar the install directory
cd ../paraview-install
tar -zcf paraview.tar.gz *

# After untarring the directory tree should like like this:
# paraview
#   |-- bin
#   |-- include
#   |-- lib
#   |-- share

# Step 5: mv to destination directory
mv paraview.tar.gz $DEST_DIR
