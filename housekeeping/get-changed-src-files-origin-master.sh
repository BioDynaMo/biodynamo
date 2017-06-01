#!/bin/bash
# Returns all source files that changed or have been added compared to origin
# master. If $1 is absolute path, returned files also have an absolute path
# Arguments:
#   $1 - Path to the project root directory

PROJECT_ROOT_DIR=$1

# Since we are comparing to origin/master, CI builds on the master branch
# will not work, because the set of changed files is always {}.
# In this case we determine the changed files using $TRAVIS_COMMIT_RANGE
if [ "$TRAVIS_BRANCH" == "master" ]; then
  FILES=$(git diff --name-only $TRAVIS_COMMIT_RANGE | grep ".*\.\(cc\|h\)$")
else
  # get changed files compared to origin/master
  FILES=$(git diff --name-only origin/master | grep ".*\.\(cc\|h\)$")
fi

# filter files that have been deleted
for f in $FILES; do
    [ -e "${PROJECT_ROOT_DIR}/${f}" ] && echo ${PROJECT_ROOT_DIR}/${f}
done
