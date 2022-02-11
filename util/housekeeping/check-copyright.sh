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
#
# Checks if the copyright notice is present in the files provide as $ARGN
# Arguments:
#   $1 - Path to the reference copyright file
#   $ARGN - Files to check
#
# Example usage from project source dir:
# ./util/housekeeping/check-copyright.sh util/housekeeping/copyright_cpp.txt \ 
#  $(util/housekeeping/get-all-src-files.sh .)

NUM_FALSE_FILES=0
COPYRIGHT_FILE=$1
shift

set -e 

for file in $@; do
  # Ingore copyrights of demo/epidemiology because of Lukas private copyright
  if [[ $file == *"demo/epidemiology/"* ]];then
    continue
  fi
  if [ "$(uname)" = 'Darwin' ]; then
    COMM_OPTIONS="-23"
  else
    COMM_OPTIONS="--nocheck-order -23"
  fi
  if [[ $(comm $COMM_OPTIONS $COPYRIGHT_FILE $file) ]]; then
    echo "$file : Licence is not correct."
    # # Uncomment this to get diff output between desired copyright and current 
    # # state.
    # head -n 16 $file > tmp.short
    # diff $COPYRIGHT_FILE tmp.short
    # rm -f tmp.short
    ((NUM_FALSE_FILES=NUM_FALSE_FILES+1))
  fi
done

if [ $NUM_FALSE_FILES -ne 0 ]; then
  echo "Error: Found $NUM_FALSE_FILES errors. Please correct."
  exit 1
else
  echo "All analysed contain the correct copyright."
fi
