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

# Pass DYLD_LIBRARY_PATH again due to OS X System Integrity Policy
if [ `uname` = "Darwin" ]; then
  source $BDM_INSTALL_DIR/bin/thisbdm.sh &> /dev/null
fi

set -e -x

tmp_dir=$(mktemp -d)
trap "rm -rf \"${tmp_dir}\"" EXIT

biodynamo demo binding_cells "${tmp_dir}"
cd "${tmp_dir}"/binding_cells

cmake .
make -j4

# start simulation
mpirun -n 2 -use-hwthread-cpus ./binding_cells --config=params.json

RETURN_CODE=$?

exit $RETURN_CODE
