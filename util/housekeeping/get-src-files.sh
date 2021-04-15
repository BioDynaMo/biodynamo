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
#   $1      - Path to the project root directory
#   $2      - Mode (either all, changed, staged)
#   $ARGN - Listing of file endings (e.g cc, h, py)
#

# Test if script receives the correct input of arguments
if [ "$#" -lt 3 ]; then
  echo "Error: $0 requires at least 3 arguments:"
  echo "Usage: $0 <path> <mode> <list_of_filetypes>"
  exit 1
fi

# Read command line arguments
PROJECT_ROOT_DIR=$1
shift
MODE=$1
shift

if [ "$MODE" == "all" ]; then 
  for filetype in $@; do
    # In comparison to previous verions, we also include the directories "util"
    # and "cli" in the search.
    find ${PROJECT_ROOT_DIR}/src ${PROJECT_ROOT_DIR}/test \
    ${PROJECT_ROOT_DIR}/cli ${PROJECT_ROOT_DIR}/util ${PROJECT_ROOT_DIR}/demo \
    -name "*.$filetype"
  done
elif [ "$MODE" == "changed" ] || [ "$MODE" == "staged" ]; then
  # Construct argument to filter git output for filetypes
  GREP_ARG=''
  for filetype in $@; do
    GREP_ARG+=$filetype
    GREP_ARG+="\|"
  done
  # get files from git
  if [ "$MODE" == "changed" ]; then
    # get all changed files respective to master
    FILES=$(git diff --name-only origin/master | grep ".*\.\(${GREP_ARG%|})$")
  else
    # get all staged files
    FILES=$(git diff --name-only --cached | grep ".*\.\(${GREP_ARG%|})$")
  fi
  # filter files that have been deleted or are in directory third_party
  for f in $FILES; do
    [ -e "${PROJECT_ROOT_DIR}/${f}" ] && echo ${PROJECT_ROOT_DIR}/${f} | \
    grep -E -v "^${PROJECT_ROOT_DIR}/third_party"
  done
else
  echo "MODE argument $MODE on second position invalid."
fi
