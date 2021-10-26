#!/bin/bash
# -----------------------------------------------------------------------------
#
# Copyright (C) 2021 CERN & University of Surrey for the benefit of the
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

# Install the prerequisites
. ${SCRIPTPATH}/util/build-third-party/third-party-prerequisites.sh

# Software versions
ROOT_VERSION=6.22.06
ROADRUNNER_VERSION=release

# root
$SCRIPTPATH/build-root.sh $ROOT_VERSION

# paraview and qt
$SCRIPTPATH/build-paraview.sh

# roadrunner
$SCRIPTPATH/build-roadrunner.sh $ROADRUNNER_VERSION
