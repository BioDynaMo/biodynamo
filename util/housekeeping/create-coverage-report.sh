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

# Creates coverage report in separate directory `coverage`
# Arguments:
#   $1 - Path to the project root directory
#   $2 - Path to the current build directory

PROJECT_ROOT_DIR=$1
BUILD_DIR=$2

set -x

pushd $BUILD_DIR
mkdir coverage 2>/dev/null
cd coverage
cmake -Dcoverage=on $PROJECT_ROOT_DIR >/dev/null
cmake --build . --target all >/dev/null
set +x
. bin/thisbdm.sh ""
set -x
cmake --build . --target coverage >/dev/null
popd
