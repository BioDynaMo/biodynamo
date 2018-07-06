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

if [[ $# -ne 0 ]]; then
  echo "ERROR: No arguments expected
  Description:
    Run installation test inside a headless docker container."
  exit 1
fi

# start x virtual framebuffer for headless environments.
export DISPLAY=:99.0
util/xvfb-initd.sh start

test/installation-test.sh
