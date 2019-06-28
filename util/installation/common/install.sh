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

if [[ $# -ne 1 ]]; then
  echo "ERROR: Wrong number of arguments.
Description:
  This script installs/updates the currently checked out version of biodynamo
Arguments:
  \$1 OS id"
  exit 1
fi

set -e

BDM_OS=$1
# remove argument so when we source prerequisites it won't complain about
# wrong number of arguments
shift

BDM_PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../../.."

# include util functions
. $BDM_PROJECT_DIR/util/installation/common/util.sh

# # install prerequisites
BDM_INSTALL_OS_SRC=$BDM_PROJECT_DIR/util/installation/$BDM_OS
# source script so BDM_INSTALL_DIR will be available in this script
. $BDM_INSTALL_OS_SRC/prerequisites.sh

# perform a clean release build
BUILD_DIR=$BDM_PROJECT_DIR/build
CleanBuild $BUILD_DIR

# print final steps
EchoSuccess "Installation of BioDynaMo finished successfully!"
EchoFinishThisStep $HOME/.bdm
