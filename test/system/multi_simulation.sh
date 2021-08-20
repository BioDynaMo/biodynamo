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


# Pass DYLD_LIBRARY_PATH again due to OS X System Integrity Policy
if [ `uname` = "Darwin" ]; then
  source $BDMSYS/bin/thisbdm.sh &> /dev/null
fi

set -e -x

SOURCE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

tmp_dir=$(mktemp -d)

cd "${tmp_dir}"
cp -r "${SOURCE}/multi_simulation" .
cd multi_simulation

cmake .
make -j4

# start simulation
mpirun -np 2 ./multi_simulation_test --config=optim.json

RETURN_CODE=$?
exit $RETURN_CODE
