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
  This script builds all third party dependencies.
  The archives will be stored in BDM_PROJECT_DIR/build
No Arguments"
  exit 1
fi

set -e -x

SCRIPTPATH="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

PARAVIEW_VERSION=v5.5.1
ROOT_VERSION=v6-13-08

# root
$SCRIPTPATH/build-root.sh $ROOT_VERSION

# paraview and qt
$SCRIPTPATH/build-paraview.sh $PARAVIEW_VERSION
