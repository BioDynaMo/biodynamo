#!/bin/bash

# exit as soon as an error occurs
set -e

if [[ $# -ne 2 ]]; then
  echo "Wrong number of arguments.
DESCRIPTION:
  Restores an archived branch.
USAGE:
  restore.sh ARCHIVE_NAME BRANCH_NAME
ARGUEMNTS:
  ARCHIVE_NAME Name of the archived branch
               Use list.sh to get a list of all archived branch names
               Use the name without 'refs/archive/' in the beginning
  BRANCH_NAME  New branch that gets created (must not exist)"
  exit 1
fi

ARCHIVE_NAME=$1
BRANCH_NAME=$2

# download all references
git fetch origin +refs/archive/*:refs/archive/*

# create new branch
git checkout -b $BRANCH_NAME refs/archive/$ARCHIVE_NAME
