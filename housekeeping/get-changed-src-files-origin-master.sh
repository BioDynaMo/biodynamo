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

# Returns all source files that changed or have been added compared to origin
# master. If $1 is absolute path, returned files also have an absolute path
# Arguments:
#   $1 - Path to the project root directory

PROJECT_ROOT_DIR=$1

# Since we are comparing to origin/master, CI builds on the master branch
# will not work, because the set of changed files is always {}.
# In this case we determine the changed files using $TRAVIS_COMMIT_RANGE
if [ "$TRAVIS_BRANCH" == "master" ]; then
  FILES=$(git diff --name-only $TRAVIS_COMMIT_RANGE | grep ".*\.\(cc\|h\)$")
else
  # get changed files compared to origin/master
  FILES=$(git diff --name-only origin/master | grep ".*\.\(cc\|h\)$")
fi

# filter files that have been deleted or are in directory third_party
for f in $FILES; do
    [ -e "${PROJECT_ROOT_DIR}/${f}" ] && echo ${PROJECT_ROOT_DIR}/${f} | \
    grep -E -v "^${PROJECT_ROOT_DIR}/third_party"
done
