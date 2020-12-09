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

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
. "${SCRIPT_DIR}/env.sh"

set -e -x

cd $SCRIPT_DIR

## Clone paraview github repository
git clone https://gitlab.kitware.com/paraview/paraview-superbuild.git superbuild
cd superbuild
git fetch origin
git submodule update --init --recursive
git checkout $PV_SUPERBUILD_VERSION
git submodule update --init --recursive

if [ "$(uname)" = "Darwin" ]; then
  git apply "$SCRIPT_DIR/paraview-superbuild-openmp.apple.patch"
else
  git apply "$SCRIPT_DIR/paraview-superbuild-openmp.patch"
fi

cd ..

git clone $PV_GIT_REPO src
cd src
git fetch origin
git checkout $PV_VERSION
git submodule update --init --recursive

