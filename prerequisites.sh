#!/bin/bash
# -----------------------------------------------------------------------------
#
# Copyright (C) 2021 CERN & Newcastle University for the benefit of the
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

# This script installs the prerequisites of BioDynaMo, but not BioDynaMo
# itself.
# Arguments:
#  $1 type of install (all or just required)
#  $2 operating system
#
# To install without a prompt set the following envar: SILENT_INSTALL=1

set -e

if [ $# -ge 3 ] || [ $# -le 0 ] || \
   [ $1 == "--help" ] || [ $1 == "-h" ]; then
  echo "
Description:
  This script installs the prerequisites for BioDynaMo,
  but not BioDynaMo itself.

Usage:
  ./prerequisites.sh <type> [<os>]

  <type>  It refers to the type of prerequistes that it is possible to
          install. You can specify two options:
          - required: install only the packages needed to build/run BioDynaMo
          - all: install all the packages (required and optional)
  <os>    Specify which OS we are targeting (optional and expert only)
"
  exit 1
fi

BDM_PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Include util functions
. $BDM_PROJECT_DIR/util/installation/common/util.sh

# Detect the OS
if [ -z $2 ]; then
    BDM_DETECTED_OS=$(DetectOs)
else
    BDM_DETECTED_OS=$2
fi

# Check if the OS the user provided is supported by the current
# version of BioDynaMo.
CheckOsSupported $BDM_PROJECT_DIR/util/installation ${BDM_DETECTED_OS}

# Check if the required install procedure is available
CheckTypeInstallSupported $1

# Compile the list of packages that will be installed
CompileListOfPackages $1 $BDM_PROJECT_DIR/util/installation ${BDM_DETECTED_OS}

EchoInfo "This script installs the following packages with sudo:"
EchoInfo ""
column ${BDM_PKG_LIST}
EchoInfo ""

if [ -z $SILENT_INSTALL ] && [ -z $GITHUB_ACTIONS ]; then
  WaitForUser
fi

EchoInfo "Installing prerequisites..." 

# Call install script for the detected OS
CallOSSpecificScript $BDM_PROJECT_DIR prerequisites.sh $1
