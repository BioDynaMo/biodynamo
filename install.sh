#!/bin/bash
# This script installs the currently checked out version of biodynamo
# Arguments:
#  $1 path to the biodynamo project directory

if [[ $# -ne 1 ]]; then
  echo "ERROR: Wrong number of arguments.
Description:
  This script installs/updates the currently checked out version of biodynamo
Arguments:
  \$1 path to the biodynamo project directory"
  exit 1
fi

set -e

BDM_PROJECT_DIR=$1

# include util functions
. $BDM_PROJECT_DIR/cmake/installation/common/util.sh

# call install script for the detected OS
CallOSSpecificScript $BDM_PROJECT_DIR install.sh $BDM_PROJECT_DIR
