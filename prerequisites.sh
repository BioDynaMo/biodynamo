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

# This script installs the the prerequisites of BioDynaMo, but not BioDynaMo
# itself. Script install.sh installs both prerequisites and BioDynaMo.
# Arguments:
#  $1 path to the biodynamo project directory

if [[ $# -ne 1 ]]; then
  echo "ERROR: Wrong number of arguments.
Description:
  This script installs the the prerequisites of BioDynaMo, but not BioDynaMo
  itself. Script install.sh installs both prerequisites and BioDynaMo.
Arguments:
  \$1 path to the biodynamo project directory"
  exit 1
fi

set -e

BDM_PROJECT_DIR=$1

# include util functions
. $BDM_PROJECT_DIR/cmake/installation/common/util.sh

# call install script for the detected OS
CallOSSpecificScript $BDM_PROJECT_DIR prerequisites.sh $BDM_PROJECT_DIR
