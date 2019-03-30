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

# This script is to prevent the test from being run by test/system-test.sh.
# This particular test is hooked up in $BDM_PROJECT_DIR/CMakeLists.txt.

set -e -x

SOURCE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

tmp_dir=$(mktemp -d)
trap "rm -rf \"${tmp_dir}\"" EXIT

cd "${tmp_dir}"
cp -r "${SOURCE}/cell_division_gpu" .
cd cell_division_gpu

# Add -Dcuda and/or -Dopencl cmake flags if available on test system
cmake .
make -j4

# start simulation
./cell_division_gpu

RETURN_CODE=$?

exit $RETURN_CODE
