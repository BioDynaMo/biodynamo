#!/bin/sh
# This script invokes the necessary Makefile targets to fix errors in a code
# submission
# Arguments:
#   $1: cmake binary directory

BINARY_DIR=$1
cmake --build $BINARY_DIR --target fetch-master
cmake --build $BINARY_DIR --target tidy
cmake --build $BINARY_DIR --target format
