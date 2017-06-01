#!/bin/bash
# Returns all source files of this project. If $1 is absolute path, returned
# files also have an absolute path
# Arguments:
#   $1 - Path to the project root directory

PROJECT_ROOT_DIR=$1
find ${PROJECT_ROOT_DIR}/src ${PROJECT_ROOT_DIR}/test ${PROJECT_ROOT_DIR}/demo -name "*.cc" -or -name "*.h"
