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

BDM_PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/.."

valgrind \
  --track-origins=yes \
  --num-callers=50 \
  --leak-resolution=high \
  --tool=memcheck \
  --leak-check=full \
  --show-leak-kinds=all \
  --gen-suppressions=all \
  --show-reachable=no \
  --suppressions=${BDM_PROJECT_DIR}/util/valgrind-bdm.supp \
  --error-exitcode=1 \
  $@
