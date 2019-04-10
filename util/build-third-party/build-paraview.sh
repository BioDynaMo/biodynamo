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

# Install prerequisites will be called inside build-qt.sh
## Install Qt using the silent JavaScript installer
. $BDM_PROJECT_DIR/util/build-third-party/build-qt.sh
cd $WORKING_DIR

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
if [ $PV_VERSION = "v5.5.2" ]; then
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

if [ `uname` = "Darwin" ]; then
  OSX_CMAKE_OPTIONS='-DPARAVIEW_DO_UNIX_STYLE_INSTALLS:BOOL=ON
                     -DCMAKE_MACOSX_RPATH:BOOL=ON
                     -DCMAKE_INSTALL_RPATH:STRING=@loader_path/../../qt/lib;@loader_path/../../../../../qt/lib;@loader_path/../lib'
fi

export CMAKE_PREFIX_PATH=/opt/ospray:$CMAKE_PREFIX_PATH
export OSPRAY_DIR=/opt/ospray
export LD_LIBRARY_PATH=/opt/ospray/lib:$LD_LIBRARY_PATH

# for release build options have a look at:
# https://gitlab.kitware.com/paraview/paraview-superbuild/blob/master/projects/paraview.cmake
Qt5_DIR=$QT_CMAKE_DIR cmake \
  -DCMAKE_INSTALL_PREFIX="../paraview-install" \
  -DCMAKE_BUILD_TYPE:STRING="Release" \
  -DPARAVIEW_ENABLE_PYTHON:BOOL=ON \
  -DPARAVIEW_INSTALL_DEVELOPMENT_FILES:BOOL=ON \
  -DPARAVIEW_USE_MPI:BOOL=:BOOL=ON \
  -DMPI_C_LIBRARIES=$MPI_LIBRARY \
  -DMPI_CXX_LIBRARIES=$MPI_LIBRARY \
  -DMPI_C_INCLUDE_PATH=$MPI_INCLUDES \
  -DMPI_CXX_INCLUDE_PATH=$MPI_INCLUDES \
  -DPARAVIEW_USE_OSPRAY:BOOL=ON \
  -DOSPRAY_INSTALL_DIR=$OSPRAY_DIR \
  $OSX_CMAKE_OPTIONS \
   ../paraview

## Step 4: compile and install
make -j$(CPUCount) install

cd ../paraview-install

# patch and bundle
if [ `uname` = "Darwin" ]; then
  ## Patch vtkkwProcessXML-pv5.5
  # make install does not set the rpath correctly on OSX
  install_name_tool -add_rpath "@loader_path/../../qt/lib" bin/vtkkwProcessXML-pv5.5
  install_name_tool -add_rpath "@loader_path/../../../../../qt/lib" bin/vtkkwProcessXML-pv5.5
  install_name_tool -add_rpath "@loader_path/../lib" bin/vtkkwProcessXML-pv5.5
fi

# Replace a string in a file
# Arguments:
#  $1 file name
#  $2 string that should be replaced
#  $3 new string
function replaceInline {
  local FILE=$1
  shift
  local SEARCH=$1
  shift
  local REPLACE=$1

  local TEMP_FILE=$(mktemp)
  sed "s|$SEARCH|$REPLACE|g" $FILE > $TEMP_FILE
  mv $TEMP_FILE $FILE
}

## patch paths in cmake files
for f in $(grep -l -R --include=*.cmake /opt/ospray .); do
  replaceInline $f "/opt/ospray" "\$ENV{OSPRAY_DIR}";
done

## bundle
cp -R /opt/ospray ospray

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
