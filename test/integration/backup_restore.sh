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

BACKUP_RESTORE_FILE="backup_restore.root"
rm $BACKUP_RESTORE_FILE

# start simulation
./backup-restore -b $BACKUP_RESTORE_FILE &

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

# restart after artificial crash
./backup-restore -b $BACKUP_RESTORE_FILE -r $BACKUP_RESTORE_FILE
RETURN_CODE=$?

rm $BACKUP_RESTORE_FILE

exit $RETURN_CODE
