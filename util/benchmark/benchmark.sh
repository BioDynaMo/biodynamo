#!/bin/bash

if [[ $# -ne 1 ]]; then
  echo "ERROR: Wrong number of arguments.
Description:
  This script creates metadata for a specific benchmark.
  The metadata will be stored a timestamped file in the current working directory
Arguments:
  \$1 The binary of the benchmark"
  exit 1
fi

if [ -z "$BDMSYS" ]; then
  echo "ERROR: Please source the BioDynaMo environmental script."
  exit 1
fi  

# Exit on error
set -e

# Create timestamped log file
binary_name=$1
current_time=$(date "+%Y.%m.%d-%H.%M.%S")
logfile=$binary_name-$current_time.log
touch $logfile

log() {
  echo -e "\n\$ $@" >> $logfile
  $@ | sed -r "s/\x1B\[([0-9]{1,2}(;[0-9]{1,2})?)?[mGK]//g" >> $logfile
}

# Record the general system info in the current working directory
CWD=`dirname "$0"`
$CWD/system-info.sh &> $logfile

# Record BioDynaMo-specific metadata
log biodynamo -v
log python3 $CWD/../../cli/util.py commit-id
log echo $OMP_NUM_THREADS

log ./$binary_name

