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

# This script installs the prerequisites of BioDynaMo, but not BioDynaMo
# itself.
# Arguments:
#  $1 operative systems
#  $2 type of install (all or just required)

set -e

if [ $# -ge 3 ] || [ $# -le 0 ]; then
  echo "ERROR: Wrong number of arguments.
Description:
  This script installs the prerequisites of BioDynaMo, but not BioDynaMo
  itself.
Usage:
  prerequisites.sh <type_of_prerequisites> [<os>]

  <type_of_prerequistes>    It refers to the type of prerequistes that it is possible to
                            install. You can specify two options:
                            - required: install only the packages needed to build/run
                              BioDynaMo;
                            - all: install all the packages (required and optional).
  <os>                      Specify which OS we are targeting (optional).
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

EchoInfo "This script install the prerequisites of BioDynaMo. For more details, please have a look at the official website:"
EchoInfo "https://biodynamo.github.io/user/installation/"
EchoInfo ""
EchoInfo "These commands require sudo rights. If you are sure that these packages have already been installed, you can skip this step."
EchoInfo ""
EchoInfo "Do you want to perform these changes? (yes/no)?"

# Ask user if she/he really wants to perform this changes
while true; do
  read -p "" yn
  case $yn in
    [Yy]* ) echo "Installing packages..." ; break;;
    [Nn]* ) echo "Aborting. No package was installed."; exit 1;;
        * ) echo "Please answer yes, no or skip.";;
  esac
done


# Call install script for the detected OS
CallOSSpecificScript $BDM_PROJECT_DIR prerequisites.sh $1
