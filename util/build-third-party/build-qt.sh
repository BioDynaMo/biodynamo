#!/bin/bash
# -----------------------------------------------------------------------------
#
# Copyright (C) 2022 CERN & University of Surrey for the benefit of the
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

BDM_PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../.."

if [[ $# -ne 0 ]]; then
  echo "ERROR: Wrong number of arguments.
Description:
  This script creates qt.tar.gz file.
  The archive will be stored in $BDM_PROJECT_DIR/build/
No Arguments"
  exit 1
fi

set -e -x

cd $BDM_PROJECT_DIR

# import util functions
. $BDM_PROJECT_DIR/util/installation/common/util.sh

# archive destination dir
DEST_DIR=$BDM_PROJECT_DIR/build
mkdir -p $DEST_DIR
EchoNewStep "Start building QT. Result will be stored in $DEST_DIR"
# working dir
WORKING_DIR=$HOME/bdm-build-third-party
mkdir -p $WORKING_DIR
cd $WORKING_DIR

QT_INSTALL_DIR=$WORKING_DIR/qt
if [ -d $QT_INSTALL_DIR ]; then
  echo "ERROR: $QT_INSTALL_DIR exists already."
  exit 1
fi

# install prerequisites
. $BDM_PROJECT_DIR/util/build-third-party/third-party-prerequisites.sh

################################################################################
# Since 2020, Qt requires you to login with an account to download the
# libraries. Instead, we download a previously built Qt directly from cernbox
################################################################################

# if [ `uname` = "Linux" ]; then
#   QT_INSTALLER=qt-installer.run
#   QT_URL="https://download.qt.io/archive/qt/5.11/5.11.0/qt-opensource-linux-x64-5.11.0.run"
#   QT_SILENT_INSTALL_JS=$BDM_PROJECT_DIR/util/build-third-party/qt-silent-install-linux.js
#   QT_LIB_PLUGINS_PARENT=$QT_INSTALL_DIR/5.11.0/gcc_64
# else
#   QT_INSTALLER=qt-installer.dmg
#   QT_URL="https://download.qt.io/archive/qt/5.11/5.11.0/qt-opensource-mac-x64-5.11.0.dmg"
#   QT_SILENT_INSTALL_JS=$BDM_PROJECT_DIR/util/build-third-party/qt-silent-install-macos.js
#   QT_LIB_PLUGINS_PARENT=$QT_INSTALL_DIR/5.11.0/clang_64
# fi

# # Download and install qt
# wget --progress=dot:giga -O $QT_INSTALLER $QT_URL
# if [ `uname` = "Linux" ]; then
#   chmod u+x $QT_INSTALLER
#   ./$QT_INSTALLER --script $QT_SILENT_INSTALL_JS --platform minimal
#   rm $QT_INSTALLER
# else
#   hdiutil attach $QT_INSTALLER
#   /Volumes/qt-opensource-mac-x64-5.11.0/qt-opensource-mac-x64-5.11.0.app/Contents/MacOS/qt-opensource-mac-x64-5.11.0 \
#     --script $QT_SILENT_INSTALL_JS \
#     --platform minimal
#   hdiutil detach /Volumes/qt-opensource-mac-x64-5.11.0
#   rm $QT_INSTALLER
# fi

# # package
# cd $QT_LIB_PLUGINS_PARENT
# tar -zcf qt-v5.11.0-$(DetectOs).tar.gz *

# # mv to destination directory
# mv qt-v5.11.0-$(DetectOs).tar.gz $DEST_DIR

################################################################################

mkdir -p $QT_INSTALL_DIR
QT_TAR="qt-v5.12.10-$(DetectOs).tar.gz"
QT_TAR_FILE="${QT_INSTALL_DIR}/${QT_TAR}"
QT_URL=http://cern.ch/biodynamo-lfs/third-party/${QT_TAR}
wget --progress=dot:giga -O $QT_TAR_FILE $QT_URL
cd ${QT_INSTALL_DIR}
tar -zxf $QT_TAR
shasum -a256 ${QT_TAR} > ${QT_TAR}.sha256
mv $QT_TAR $QT_TAR.sha256 $DEST_DIR
