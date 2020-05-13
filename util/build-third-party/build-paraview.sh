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

#####################################
## Building ParaView for BioDynaMo ##
#####################################
WORKING_DIR=~/bdm-build-third-party/paraview 
PV_FLAVOR="default"

function printUsageAndExit {
  echo "
  $0 [-w path-to-working-dir -f paraview-flavor -p]
Description:
  This script builds paraview.
  The archive will be stored in $BDM_PROJECT_DIR/build
Arguments:
  -w
    working dir path
    Default value: $WORKING_DIR
  -f
    paraview flavour. Possible values: default or nvidia-headless
    Default value: $WORKING_DIR
  -p
    Install prerequisites. Skip this step if -p is missing"
  exit 1
}

# parse options
while getopts ":w:f:p" opt; do
  case ${opt} in
    w )
      WORKING_DIR=$OPTARG
      ;;
    f )
      PV_FLAVOR=$OPTARG
      ;;
    p )
      INSTALL_PREREQUISITES=1
      ;;
    \? )
      echo "Invalid option: $OPTARG" 1>&2
      printUsageAndExit
      ;;
    : )
      echo "Invalid option: $OPTARG requires an argument" 1>&2
      printUsageAndExit
      ;;
  esac
done
shift $((OPTIND -1))

BDM_PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../.."
cd $BDM_PROJECT_DIR

set -e -x

mkdir -p "$WORKING_DIR"

if [ ! -z "$(ls -A \"$WORKING_DIR\")" ]; then
  echo "ERROR: Working directory ($WORKING_DIR) is not empty"
  exit 2
fi

cp "$BDM_PROJECT_DIR/util/build-third-party/paraview/"* "$WORKING_DIR"
cd $WORKING_DIR

# Prepend parameters to env.sh so scripts can be called again at a later stage
# This helps debugging an error at one stage.
echo "export BDM_PROJECT_DIR=$BDM_PROJECT_DIR" > env.sh
echo "export WORKING_DIR=$WORKING_DIR" >> env.sh
echo "export PV_FLAVOR=$PV_FLAVOR" >> env.sh
cat "$BDM_PROJECT_DIR/util/build-third-party/paraview/env.sh" >> env.sh

if [ ! -z "$INSTALL_PREREQUISITES" ]; then
  ./prerequisites.sh
fi
./checkout-code.sh
./build.sh
./package.sh

