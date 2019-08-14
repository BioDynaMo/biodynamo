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

# speed-up build by disabling tests and demos
export BDM_CMAKE_FLAGS="-Dtest=off"

# Build BioDynaMo
$BDM_PROJECT_DIR/install.sh << EOF
y
EOF

# reload shell and source biodynamo
set +e +x
source ~/biodynamo/bin/thisbdm.sh
set -e -x

# run system test.
test/system-test.sh

# verify if out of source builds work
cd ~
biodynamo new test-sim
run_cmake_simulation test-sim
