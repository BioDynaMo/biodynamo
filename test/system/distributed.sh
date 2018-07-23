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

set -e -x

tmp_dir=$(mktemp -d)
trap "rm -rf \"${tmp_dir}\"" EXIT

biodynamo demo distributed "${tmp_dir}"
cd "${tmp_dir}/distributed"
mkdir build
cd build
cmake ..
make
python ../driver.py -l libdistributed-ray.so
