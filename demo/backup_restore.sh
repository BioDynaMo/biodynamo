#!/bin/bash

BACKUP_RESTORE_FILE="backup_restore.root"

# start simulation
./backup-restore -b $BACKUP_RESTORE_FILE &

# simulate crash of simulation after 4 seconds
SIMULATION_PID=$!
sleep 4
kill -9 $SIMULATION_PID

# restart after artificial crash
./backup-restore -b $BACKUP_RESTORE_FILE -r $BACKUP_RESTORE_FILE
RETURN_CODE=$?

rm $BACKUP_RESTORE_FILE

exit $RETURN_CODE
