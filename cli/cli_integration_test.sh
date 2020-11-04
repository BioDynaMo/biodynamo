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

echo "Starting BioDynaMo CLI Test"

cd /tmp
biodynamo new test-sim
cd test-sim
biodynamo build
biodynamo run >actual 2>&1

echo "Warning in <InitializeBiodynamo>: Config file bdm.toml not found in `.` or `../` directory." > expected
echo "Warning: No backup file name given. No backups will be made!" >> expected
echo "Your agents are getting near the edge of the simulation space. Be aware of boundary conditions that may come into play!" >> expected
echo "Your agents are getting near the edge of the simulation space. Be aware of boundary conditions that may come into play!" >> expected

diff expected actual
exit $?
