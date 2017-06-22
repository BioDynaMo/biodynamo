#!/bin/sh
# This script invokes the necessary Makefile targets to check a code submission
# Arguments:
#   $1: cmake binary directory

BINARY_DIR=$1
RETURN_VALUE=0
LAST_RET_VAL=0

cmake --build $BINARY_DIR --target all
LAST_RET_VAL=$?
if [ $LAST_RET_VAL != 0 ]; then
  RETURN_VALUE=$LAST_RET_VAL
fi
cmake --build $BINARY_DIR --target check
LAST_RET_VAL=$?
if [ $LAST_RET_VAL != 0 ]; then
  RETURN_VALUE=$LAST_RET_VAL
fi
cmake --build $BINARY_DIR --target fetch-master
cmake --build $BINARY_DIR --target check-format
if [ $? != 0 ]; then
  cmake --build $BINARY_DIR --target show-format
fi
cmake --build $BINARY_DIR --target check-tidy
if [ $? != 0 ]; then
  cmake --build $BINARY_DIR --target show-tidy
fi
cmake --build $BINARY_DIR --target check-cpplint
cmake --build $BINARY_DIR --target doc
if ! [ $TRAVIS ]; then
  cmake --build $BINARY_DIR --target coverage-build
fi
exit $RETURN_VALUE
