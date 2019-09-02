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

BDM_PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../.."

. "$BDM_PROJECT_DIR/util/installation/common/util.sh"
. "$BDM_PROJECT_DIR/test/util.inc"

# Currently SBML is not supported on osx -> skip this test on osx
BDM_OS=$(DetectOs)
if [ "$BDM_OS" = "osx" ] || [ "$BDM_OS" = "travis-osx" ]; then
  exit 0
fi

demo_name="sbml_integration"
demo_dir=$(mktemp -d)
biodynamo demo "${demo_name}" "${demo_dir}"
run_cmake_simulation "${demo_dir}/${demo_name}"
rm -rf "${demo_dir}"
