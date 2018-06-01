#!/bin/sh
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
cmake --build $BINARY_DIR --target show-format
cmake --build $BINARY_DIR --target show-tidy
cmake --build $BINARY_DIR --target check-cpplint
cmake --build $BINARY_DIR --target doc
if ! [ $TRAVIS ]; then
  cmake --build $BINARY_DIR --target coverage-build
fi
exit $RETURN_VALUE
