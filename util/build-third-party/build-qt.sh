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

if [[ $# -ne 0 ]]; then
  echo "ERROR: Wrong number of arguments.
Description:
  This script creates qt.tar.gz file.
  The archive will be stored in BDM_PROJECT_DIR/build/qt.tar.gz
No Arguments"
  exit 1
fi

set -e -x

BDM_PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../.."
cd $BDM_PROJECT_DIR

# import util functions
. $BDM_PROJECT_DIR/util/installation/common/util.sh

# archive destination dir
DEST_DIR=$BDM_PROJECT_DIR/build
mkdir -p $DEST_DIR
EchoNewStep "Start building QT. Result will be stored in $DEST_DIR"
# working dir
WORKING_DIR=~/bdm-build-third-party
mkdir -p $WORKING_DIR
cd $WORKING_DIR

QT_INSTALL_DIR=$WORKING_DIR/qt
if [ -d $QT_INSTALL_DIR ]; then
  echo "ERROR: $QT_INSTALL_DIR exists already."
  exit 1
fi

# install prerequisites
. $BDM_PROJECT_DIR/util/build-third-party/third-party-prerequisites.sh

if [ `uname` = "Linux" ]; then
  QT_INSTALLER=qt-installer.run
  QT_URL="https://download.qt.io/archive/qt/5.11/5.11.0/qt-opensource-linux-x64-5.11.0.run"
  QT_SILENT_INSTALL_JS=$BDM_PROJECT_DIR/util/build-third-party/qt-silent-install-linux.js
  QT_LIB_PLUGINS_PARENT=$QT_INSTALL_DIR/5.11.0/gcc_64
else
  QT_INSTALLER=qt-installer.dmg
  QT_URL="https://download.qt.io/archive/qt/5.11/5.11.0/qt-opensource-mac-x64-5.11.0.dmg"
  QT_SILENT_INSTALL_JS=$BDM_PROJECT_DIR/util/build-third-party/qt-silent-install-macos.js
  QT_LIB_PLUGINS_PARENT=$QT_INSTALL_DIR/5.11.0/clang_64
fi

# Download and install qt
wget --progress=dot:giga -O $QT_INSTALLER $QT_URL
if [ `uname` = "Linux" ]; then
  chmod u+x $QT_INSTALLER
  ./$QT_INSTALLER --script $QT_SILENT_INSTALL_JS -platform minimal
  rm $QT_INSTALLER
else
  hdiutil attach $QT_INSTALLER
  /Volumes/qt-opensource-mac-x64-5.11.0/qt-opensource-mac-x64-5.11.0.app/Contents/MacOS/qt-opensource-mac-x64-5.11.0 \
    --script $QT_SILENT_INSTALL_JS \
    -platform minimal
  hdiutil detach /Volumes/qt-opensource-mac-x64-5.11.0
  rm $QT_INSTALLER
fi

# package
cd $QT_LIB_PLUGINS_PARENT
tar -zcf qt.tar.gz *

# mv to destination directory
mv qt.tar.gz $DEST_DIR
