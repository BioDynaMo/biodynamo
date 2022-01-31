#!/bin/bash
# -----------------------------------------------------------------------------
#
# Copyright (C) 2022 CERN & University of Surrey for the benefit of the
# BioDynaMo collaboration. All Rights Reserved.
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

FILES=$(git diff --name-only origin/master | grep ".*\.\(cc\|h\)$")

# filter files that have been deleted or are in directory third_party
for f in $FILES; do
    [ -e "${PROJECT_ROOT_DIR}/${f}" ] && echo ${PROJECT_ROOT_DIR}/${f} | \
    grep -E -v "^${PROJECT_ROOT_DIR}/third_party"
done
