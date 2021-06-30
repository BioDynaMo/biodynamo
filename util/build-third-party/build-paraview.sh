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

#####################################
## Building ParaView for BioDynaMo ##
#####################################
WORKING_DIR="$HOME/bdm-build-third-party/paraview"
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
    Default value: $PV_FLAVOR
  -d
    ignore dirty working dir
  -p
    install prerequisites. Skip this step if -p is missing
  -C
    skip checkout-code.sh
  -Q
    skip prerequisite Qt5 download and install
  -M
    don't install prerequisite packages using the system's package manager
  -P
    don't install pyenv prerequisite
  -Z
    skip package.sh"
  exit 1
}

# parse options
while getopts ":w:f:pdCMPQZ" opt; do
  case ${opt} in
    w )
      WORKING_DIR=$(realpath $OPTARG)
      ;;
    f )
      PV_FLAVOR=$OPTARG
      ;;
    p )
      INSTALL_PREREQUISITES=1
      ;;
    d )
      DIRTY_DIR=1
      ;;
    C )
      SKIP_CHECKOUT=1
      ;;
    M )
      SKIP_PACKAGE_MAN=1
      ;;
    P )
      SKIP_PYENV=1
      ;;
    Q )
      SKIP_QT=1
      ;;
    Z )
      SKIP_PACKAGING=1
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

if [ -z "$DIRTY_DIR" ] && [ -n "$(ls -A \"$WORKING_DIR\")" ]; then
  echo "ERROR: Working directory ($WORKING_DIR) is not empty"
  exit 2
fi

cp "$BDM_PROJECT_DIR/util/build-third-party/paraview/"* "$WORKING_DIR"
cd $WORKING_DIR

# Prepend parameters to env.sh so scripts can be called again at a later stage
# This helps debugging an error at one stage.
echo "export BDM_PROJECT_DIR=$BDM_PROJECT_DIR" > env.sh
echo "export WORKING_DIR=$WORKING_DIR" >> env.sh
echo "export SKIP_QT=$SKIP_QT" >> env.sh
echo "export SKIP_PACKAGE_MAN=$SKIP_PACKAGE_MAN" >> env.sh
echo "export SKIP_PYENV=$SKIP_PYENV" >> env.sh
echo "export PV_FLAVOR=$PV_FLAVOR" >> env.sh
cat "$BDM_PROJECT_DIR/util/build-third-party/paraview/env.sh" >> env.sh

if [ -n "$INSTALL_PREREQUISITES" ]; then
  ./prerequisites.sh
fi

if [ -z "$SKIP_CHECKOUT" ]; then
  ./checkout-code.sh
fi

./build.sh

if [ -z "$SKIP_PACKAGING" ]; then
  ./package.sh
fi
