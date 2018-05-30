#!/bin/bash

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
$BDM_PROJECT_DIR/cmake/installation/common/install.sh . travis-osx
