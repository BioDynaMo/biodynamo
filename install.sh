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

if [ $# -ge 2 ]; then
  echo "ERROR: Wrong number of arguments.
Usage:
  install.sh [<os>]
Description:
  This script installs/updates the currently checked out version of biodynamo
Arguments:
  <os>  Specify manually which operating system is being used."
  exit 1
fi

set -e

BDM_PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# include util functions
. $BDM_PROJECT_DIR/util/installation/common/util.sh

# Detect the OS (or get the one the user specified)
if [ -z $1 ]; then
    BDM_DETECTED_OS=$(DetectOs)
else
    BDM_DETECTED_OS=$1
fi

# Install all prerequisites
$BDM_PROJECT_DIR/prerequisites.sh all ${BDM_DETECTED_OS}
if [ $? != 0 ]; then
   exit 1
fi

# call install script for the detected OS
$BDM_PROJECT_DIR/util/installation/common/install.sh ${BDM_DETECTED_OS}
