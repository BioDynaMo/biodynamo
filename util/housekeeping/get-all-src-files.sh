#!/bin/bash
# -----------------------------------------------------------------------------
#
# Copyright (C) 2021 CERN & Newcastle University for the benefit of the
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

# Returns all source files of this project. If $1 is absolute path, returned
# files also have an absolute path
# Arguments:
#   $1 - Path to the project root directory

PROJECT_ROOT_DIR=$1
find ${PROJECT_ROOT_DIR}/src ${PROJECT_ROOT_DIR}/test ${PROJECT_ROOT_DIR}/demo -name "*.cc" -or -name "*.h"
