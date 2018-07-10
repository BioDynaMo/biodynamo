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

if [ $# -ne 0 ]; then
  echo "Wrong number of arguments.
Description:
  Run system tests in a BioDynaMo environment.
Usage:
  system-test.sh
No Arguments
  "
  exit 1
fi

set -e -x

source $(dirname "$BASH_SOURCE[0]")/util.inc

for demo_name in ${CMAKE_DEMOS[@]}  # No quotation.
do
  specialized_test="${BDM_PROJECT_DIR}/test/system/${demo_name}.sh"
  if [ -f "${specialized_test}" ]; then
    # We have a specialized test, run it.
    "${specialized_test}"
  else
    # Otherwise, just build and run with biodynamo run.
    demo_dir=$(mktemp -d)
    biodynamo demo "${demo_name}" "${demo_dir}"
    run_cmake_simulation "${demo_dir}/${demo_name}"
    rm -rf "${demo_dir}"
  fi
done

# Other specialized tests.
"${BDM_PROJECT_DIR}/test/system/backup_restore.sh"
"${BDM_PROJECT_DIR}/test/system/makefile_project.sh"

exit $?
