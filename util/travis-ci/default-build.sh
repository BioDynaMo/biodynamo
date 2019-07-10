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

if [[ $# -ne 1 ]]; then
  echo "Wrong number of arguments.
Description:
  Run a travis default_build
Usage:
  default-build.sh <os-name>
Arguments:
  <os-name> Name of the current operative system
"
  exit 1
fi

BDM_PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../.."

set -e -x

echo ${TRAVIS_OS_NAME}

# start x virtual framebuffer for headless environments.
#export DISPLAY=:99.0
#util/xvfb-initd.sh start

# git describe does not work if last commit tag is not checked out
git fetch --unshallow || true
git fetch --tags origin || true

python --version || true
python3 --version || true

# add master branch
# https://github.com/travis-ci/travis-ci/issues/6069
git remote set-branches --add origin master

# Install all the dependencies
$BDM_PROJECT_DIR/prerequisites.sh $1 all << EOF
y
EOF

# Build BioDynaMo
mkdir build
cd build
cmake ../
make -j 4

# output compiler information
echo ${CXX}
${CXX} --version || true
${CXX} -v || true

cd build
ctest -V

if [ "$TRAVIS_BRANCH" = "master" ] && [ "$TRAVIS_OS_NAME" = "linux" ] && [ "$TRAVIS_PULL_REQUEST" = "false" ]; then
  ../util/travis-ci/deploy.sh
fi
