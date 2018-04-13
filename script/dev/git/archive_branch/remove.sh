#!/bin/bash

# exit as soon as an error occurs
set -e

if [[ $# -ne 1 ]]; then
  echo "Wrong number of arguments.
DESCRIPTION:
  Removes an archived branch locally and from origin
USAGE:
  remove.sh ARCHIVE_NAME
ARGUEMNTS:
  ARCHIVE_NAME Name of the archived branch
               Use list.sh to get a list of all archived branch names
               Use the name without 'refs/archive/' in the beginning"
  exit 1
fi

ARCHIVE_NAME=$1

GIT_FOLDER_PATH=$(git rev-parse --show-toplevel)/.git

echo "Remove locally"
rm ${GIT_FOLDER_PATH}/refs/archive/${ARCHIVE_NAME}

echo "Remove from origin"
git push origin :refs/archive/${ARCHIVE_NAME}
