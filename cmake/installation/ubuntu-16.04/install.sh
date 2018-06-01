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
  This script installs the currently checked out version of biodynamo
Arguments:
  \$1 path to the biodynamo project directory"
  exit 1
fi

set -e

BDM_PROJECT_DIR=$1

# no OS specifics -> use common install script
$BDM_PROJECT_DIR/cmake/installation/common/install.sh . ubuntu-16.04
