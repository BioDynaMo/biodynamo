#!/bin/sh
# This script invokes the necessary Makefile targets to check a code submission
# Arguments:
#   $1: cmake binary directory

BINARY_DIR=$1
RETURN_VALUE=0

cmake --build $BINARY_DIR --target all
if [ $? != 0 ]; then
  RETURN_VALUE=$?
fi
cmake --build $BINARY_DIR --target check
if [ $? != 0 ]; then
  RETURN_VALUE=$?
fi
cmake --build $BINARY_DIR --target fetch-master
cmake --build $BINARY_DIR --target check-format
if [ $? != 0 ]; then
  cmake --build $BINARY_DIR --target show-format
fi
cmake --build $BINARY_DIR --target check-clang-tidy
if [ $? != 0 ]; then
  cmake --build $BINARY_DIR --target show-clang-tidy
fi
cmake --build $BINARY_DIR --target check-cpplint
cmake --build $BINARY_DIR --target doc
if ! [ $TRAVIS ]; then
  cmake --build $BINARY_DIR --target coverage-build
fi
exit $RETURN_VALUE
