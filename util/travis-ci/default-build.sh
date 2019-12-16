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
  Run a travis default_build
Usage:
  default-build.sh
Arguments:
  No Arguments
"
  exit 1
fi

BDM_PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../.."

set -e -x

echo ${TRAVIS_OS_NAME}

# Source some utils functions
. $BDM_PROJECT_DIR/util/installation/common/util.sh

# start x virtual framebuffer for headless environments.
export DISPLAY=:99.0
util/xvfb-initd.sh start

# git describe does not work if last commit tag is not checked out
git fetch --unshallow || true
git fetch --tags origin || true

python --version || true
python3 --version || true

# add master branch
# https://github.com/travis-ci/travis-ci/issues/6069
git remote set-branches --add origin master

# Install all the dependencies
$BDM_PROJECT_DIR/prerequisites.sh all << EOF
y
EOF

# Custom instruction for MacOS (just in case)
# Export path to make cmake find LLVM's clang (otherwise OpenMP won't work)
BDM_OS=$(DetectOs)
if [ ${BDM_OS} = "osx" ] || [ ${BDM_OS} = "travis-osx" ]; then
    if [ -z ${CXX} ] && [ -z ${CC} ] ; then
        if [ -x "/usr/local/opt/llvm/bin/clang++" ]; then
            export LLVMDIR="/usr/local/opt/llvm"
            export CC=$LLVMDIR/bin/clang
            export CXX=$LLVMDIR/bin/clang++
            export CXXFLAGS=-I$LLVMDIR/include
            export LDFLAGS=-L$LLVMDIR/lib
            export PATH=$LLVMDIR/bin:$PATH
        elif [ -x "/opt/local/bin/clang++-mp-8.0" ]; then
            export CC=/opt/local/bin/clang++-mp-8.0
            export CXX=/opt/local/bin/clang-mp-8.0
        elif [ -x "/sw/opt/llvm-5.0/bin/clang++" ]; then
            export CC=/sw/opt/llvm-5.0/bin/clang++
            export CXX=/sw/opt/llvm-5.0/bin/clang
        fi
    fi
fi

# Build BioDynaMo
mkdir build
cd build
cmake -Dwebsite=on ..
make -j$(CPUCount)

# output compiler information
echo ${CXX}
${CXX} --version || true
${CXX} -v || true

ctest -V

../util/travis-ci/deploy.sh
