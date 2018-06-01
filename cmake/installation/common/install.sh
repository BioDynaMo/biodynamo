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

if [[ $# -ne 2 ]]; then
  echo "ERROR: Wrong number of arguments.
Description:
  This script installs the currently checked out version of biodynamo
Arguments:
  \$1 path to the biodynamo project directory
  \$2 OS id"
  exit 1
fi

set -e

BDM_PROJECT_DIR=$1
BDM_OS=$2

# include util functions
. $BDM_PROJECT_DIR/cmake/installation/common/util.sh

# check if this script is run with sudo
RequireSudo

# remove whole install dir
BDM_INSTALL_DIR=/opt/biodynamo
if [ ! -d "$BDM_INSTALL_DIR" ]; then
  sudo rm -rf $BDM_INSTALL_DIR
fi

# install prerequisites
BDM_INSTALL_OS_SRC=$BDM_PROJECT_DIR/cmake/installation/$BDM_OS
$BDM_INSTALL_OS_SRC/prerequisites.sh $BDM_PROJECT_DIR

# reload shell and source biodynamo
set +e
. $(BashrcFile)
$use_biodynamo
set -e

# perform a clean release build
BUILD_DIR=$BDM_PROJECT_DIR/build
CleanBuild $BUILD_DIR


echo "Installation of BioDynaMo finished successfully!"
EchoFinishThisStep
