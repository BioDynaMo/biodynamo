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
  This script creates libzmqpp.tar.gz file.
  The archive will be stored in BDM_PROJECT_DIR/build/libzmqpp.tar.gz
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
EchoNewStep "Start building libzmqpp. Result will be stored in $DEST_DIR"
# working dir
WORKING_DIR=~/bdm-build-third-party
mkdir -p $WORKING_DIR
cd $WORKING_DIR

INSTALL_DIR=$WORKING_DIR/libzmqpp
if [ -d $INSTALL_DIR ]; then
  echo "ERROR: $INSTALL_DIR exists already."
  exit 1
fi

# install prerequisites
. $BDM_PROJECT_DIR/util/build-third-party/third-party-prerequisites.sh

# Download and install.
PN=libzmqpp
VERSION=4.2.0
SUFFIX=.tar.gz
DEST_FILE="${PN}-${VERSION}.${SUFFIX}"
SRC_URL="https://github.com/zeromq/zmqpp/archive/${VERSION}.zip"
wget --progress=dot:giga -O "${DEST_FILE}" "${SRC_URL}"
if [ `uname` = "Linux" ]; then
  unzip "${DEST_FILE}"
  cd "zmqpp-${VERSION}"
  mkdir -p build
  cd build
  cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" ../
  make install
else
  echo "macOS is not supported, yet."
  exit 1
fi

# package
cd "${INSTALL_DIR}"
tar -zcf "${PN}.tar.gz" *

# mv to destination directory
mv "${PN}.tar.gz" $DEST_DIR
