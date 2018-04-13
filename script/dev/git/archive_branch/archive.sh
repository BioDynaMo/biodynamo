#!/bin/bash

# exit as soon as an error occurs
set -e

if [[ $# -eq 1 ]]; then
  BRANCH_NAME=$1
  ARCHIVE_NAME=$1
elif [[ $# -eq 2 ]]; then
  BRANCH_NAME=$1
  ARCHIVE_NAME=$2
else
  echo "Wrong number of arguments.
DESCRIPTION:
  Archives a branch and pushes it to origin. The branch is not removed locally
  nor from origin.
USAGE:
  archive.sh BRANCH_NAME [ARCHIVE_NAME]
ARGUEMNTS:
  BRANCH_NAME Branch name that should be archived
  ARCHIVE_NAME Name of the archived branch (Default value is BRANCH_NAME)"
  exit 1
fi

GIT_FOLDER_PATH=$(git rev-parse --show-toplevel)/.git

# check if file already exists
if [ -f ${GIT_FOLDER_PATH}/refs/archive/${ARCHIVE_NAME} ]; then
   echo "$ARCHIVE_NAME already exists. Use a different name."
   exit 1
fi

# create archive subdirectory if it does not exist
DIR=$(dirname ${GIT_FOLDER_PATH}/refs/archive/$ARCHIVE_NAME)
mkdir -p $DIR

git tag archive/${BRANCH_NAME} ${BRANCH_NAME}
cp ${GIT_FOLDER_PATH}/refs/tags/archive/${BRANCH_NAME} ${GIT_FOLDER_PATH}/refs/archive/${ARCHIVE_NAME}
git tag -d archive/${BRANCH_NAME} 2>&1 >/dev/null

echo "Push archived branch to origin"
git push origin refs/archive/$ARCHIVE_NAME
