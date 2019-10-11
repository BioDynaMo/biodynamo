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
  echo "Wrong number of arguments.
Description:
  Check code style
Usage:
  check-code-style.sh
No Arguments
"
  exit 1
fi

BDM_PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../.."

set -e -x

echo ${TRAVIS_OS_NAME}

# git describe does not work if last commit tag is not checked out
git fetch --unshallow || true
git fetch --tags

python --version || true
python3 --version || true

# add master branch
# https://github.com/travis-ci/travis-ci/issues/6069
git remote set-branches --add origin master
git fetch

$BDM_PROJECT_DIR/prerequisites.sh all << EOF
y
EOF

# reload shell and source biodynamo
set +e +x
source ~/biodynamo/bin/thisbdm.sh
set -e -x

# print lines-of-code statistic
sudo apt install -y cloc
cloc --exclude-dir=build .

mkdir build || true
cd build
cmake ..
cmake --build . --target fetch-master
cmake --build . --target gtest || true
cmake --build . --target show-format || true
cmake --build . --target show-tidy || true
cmake --build . --target check-cpplint || true
