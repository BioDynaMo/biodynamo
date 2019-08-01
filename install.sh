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
  This script installs/updates the currently checked out version of biodynamo
No Arguments"
  exit 1
fi

set -e

BDM_PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BDM_DETECTED_OS=$(DetectOs)

# include util functions
. $BDM_PROJECT_DIR/util/installation/common/util.sh

# Install all prerequisites
$BDM_PROJECT_DIR/prerequisites.sh ${BDM_DETECTED_OS} all

# call install script for the detected OS
util/installation/common/install.sh ${BDM_DETECTED_OS}
