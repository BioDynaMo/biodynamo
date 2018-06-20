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

if [[ $# -ne 1 ]]; then
  echo "Wrong number of arguments.
Description:
  Run a travis installation test
Usage:
  installation-test.sh OS
Arguments:
  OS OS id of the container
  "
  exit 1
fi

BDM_OS=$1
BDM_PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../.."

set -e -x

# git describe does not work if last commit tag is not checked out
git fetch --unshallow &>/dev/null || true

if [ $BDM_OS != "osx" ]; then
  util/run-inside-docker.sh $BDM_OS test/installation-test.sh
else
  if [ `uname` != "Darwin" ]; then
    echo "ERROR: Installation tests for OSX can only be done on an OSX operating system"
    exit 1
  fi
  git config --system user.name "Test User"
  git config --system user.email user@test.com
  # hide the fact that this is running on travis so DetectOs detects "osx"
  unset TRAVIS
  # don't unset TRAVIS_OS_NAME, because it is needed by the ReuqireSudo workaround
  test/installation-test.sh
fi
