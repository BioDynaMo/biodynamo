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

set -x

temp_dir=$(mktemp -d)
trap "rm -rf \"${temp_dir}\"" EXIT

biodynamo demo makefile_project "${temp_dir}"
cd "${temp_dir}/makefile_project"
make clean

if [ `uname` = "Darwin" ]; then
  # following line creates the cache for bdm-config
  # without it the make call fails on Travis OSX
  bdm-config --cxxflag
fi

make
./my-simulation 2>/dev/null | grep '^Simulation completed successfully!$'
exit $?
