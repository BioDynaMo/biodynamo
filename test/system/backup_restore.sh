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


# Pass DYLD_LIBRARY_PATH again due to OS X System Integrity Policy
if [ `uname` = "Darwin" ]; then
  source $BDMSYS/bin/thisbdm.sh &> /dev/null
fi

set -e -x

SOURCE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

tmp_dir=$(mktemp -d)
trap "rm -rf \"${tmp_dir}\"" EXIT

cd "${tmp_dir}"
cp -r "${SOURCE}/backup_restore" .
cd backup_restore

cmake .
make -j4

BACKUP_RESTORE_FILE="backup_restore.root"

# start simulation
./backup_restore -b $BACKUP_RESTORE_FILE &

# simulate crash of simulation after 5 seconds
SIMULATION_PID=$!
sleep 5
# wait longer if backup file has not been created yet
# check every second if backup file exists
for i in {0..5}; do
  if [ -e "$BACKUP_RESTORE_FILE" ]; then
    break
  fi
  sleep 1
done
kill -9 $SIMULATION_PID
sleep 1

# restart after artificial crash
./backup_restore -r $BACKUP_RESTORE_FILE
RETURN_CODE=$?

exit $RETURN_CODE
