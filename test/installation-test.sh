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

if [ $# -ne 0 ]; then
  echo "Wrong number of arguments.
Description:
  Run installation tests
Usage:
  installation-test.sh
No Arguments
  "
  exit 1
fi

set -e -x

source $(dirname "$BASH_SOURCE[0]")/util.inc

cd $BDM_PROJECT_DIR
# Currently we are inside the biodynamo project directory, mapped as volume
# from the host

# import util functions
. $BDM_PROJECT_DIR/util/installation/common/util.sh

BDM_OS=$(DetectOs)

NOTEBOOK_FLAG="-Dnotebooks=off"

# speed-up build by disabling tests and demos
# Currently SBML and notebooks is not supported on osx
if [ "$BDM_OS" != "osx" ]; then
  SBML_FLAG="-Dsbml=on"
  NOTEBOOK_FLAG="-Dnotebooks=on"
fi
export BDM_CMAKE_FLAGS="-Dtest=off ${NOTEBOOK_FLAG} ${SBML_FLAG}"

# Build BioDynaMo
SILENT_INSTALL=1 $BDM_PROJECT_DIR/install.sh

BDM_VERSION=$(cat $BUILD_DIR/version/bdm_shortversion)
INSTALL_DIR=${HOME}/biodynamo-v${BDM_VERSION}

# reload shell and source biodynamo
set +e +x
source ${INSTALL_DIR}/bin/thisbdm.sh
set -e -x

# run system test.
test/system-test.sh

# verify if out of source builds work
cd ~
biodynamo new test-sim
run_cmake_simulation test-sim
