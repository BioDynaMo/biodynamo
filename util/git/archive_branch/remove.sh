#!/bin/bash
# -----------------------------------------------------------------------------
#
# Copyright (C) 2021 CERN & University of Surrey for the benefit of the
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

# exit as soon as an error occurs
set -e

if [[ $# -ne 1 ]]; then
  echo "Wrong number of arguments.
DESCRIPTION:
  Removes an archived branch locally and from origin
USAGE:
  remove.sh ARCHIVE_NAME
ARGUEMNTS:
  ARCHIVE_NAME Name of the archived branch
               Use list.sh to get a list of all archived branch names
               Use the name without 'refs/archive/' in the beginning"
  exit 1
fi

ARCHIVE_NAME=$1

GIT_FOLDER_PATH=$(git rev-parse --show-toplevel)/.git

echo "Remove locally"
rm ${GIT_FOLDER_PATH}/refs/archive/${ARCHIVE_NAME}

echo "Remove from origin"
git push origin :refs/archive/${ARCHIVE_NAME}
