#!/bin/bash
# Creates coverage report in separate directory `coverage`
# Arguments:
#   $1 - Path to the project root directory
#   $2 - Path to the current build directory

PROJECT_ROOT_DIR=$1
BUILD_DIR=$2

pushd $BUILD_DIR
mkdir coverage 2>/dev/null
cd coverage
cmake -Dcoverage=on $PROJECT_ROOT_DIR >/dev/null
cmake --build . --target runBiodynamoTests >/dev/null
cmake --build . --target coverage >/dev/null
popd
