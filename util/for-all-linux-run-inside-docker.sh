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

if [[ $# -lt 1 ]]; then
  echo "Wrong number of arguments.
Description:
  Run a script inside a docker container for all supported linux distributions
Usage:
  $0 SCRIPT [SCRIPT_ARGUMENTS]
Arguments:
  SCRIPT absolute path to script that should be executed inside the container
         or relative path to BDM_PROJECT_DIR.
         NB: In both cases the script must be inside BDM_PROJECT_DIR
  SCRIPT_ARGUMENTS arguments that are passed to the script inside the docker
                   container (optional)
  "
  exit 1
fi

set -x

BDM_PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/.."

LOG=$(mktemp)

for system in "ubuntu-16.04" "ubuntu-18.04" "centos-7"; do
  $BDM_PROJECT_DIR/util/run-inside-docker.sh $system $@ 2>&1 | tee  -a $LOG
done
echo "Log was written to $LOG"
