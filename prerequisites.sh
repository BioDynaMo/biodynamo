#!/bin/bash
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
