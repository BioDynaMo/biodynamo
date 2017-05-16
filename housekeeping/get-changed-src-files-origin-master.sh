#!/bin/bash
# Returns all source files that changed or have been added compared to origin
# master. If $1 is absolute path, returned files also have an absolute path
# Arguments:
#   $1 - Path to the project root directory

PROJECT_ROOT_DIR=$1

FILES=$(git diff --name-only origin/master | grep ".*\.\(cc\|h\)$")
# filter files that have been deleted
for f in $FILES; do
    [ -e "${PROJECT_ROOT_DIR}/${f}" ] && echo ${PROJECT_ROOT_DIR}/${f}
done
