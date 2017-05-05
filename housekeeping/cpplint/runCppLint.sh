#!/bin/bash

SCRIPTPATH="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

function help {
  echo "runCppLint.py help|--help|-h|<mode>|<wildcard>
  Executes cpplint on a filtered subset of *.h and *.cc files in directory
  include, source and test.
  help|--help|-h
    Print this help message
  <mode>
     defines the filter condition options:
     staged: only files that are staged in git are checked
     master: all files that have been changed compared to the local
             master HEAD are checked
  <wildcard>
     single file name or wildcard like *.cc or *.h"
  exit 0
}

git_cmd=""
if [[ "$1" == "help" || "$1" == "--help" || "$1" == "-h" ]] ; then
  help
elif [ "$1" == "staged" ]; then
  git_cmd="git --no-pager diff --name-only --cached"
elif [ "$1" == "master" ]; then
  git_cmd="git --no-pager diff --name-only FETCH_HEAD $(git merge-base FETCH_HEAD master)"
elif [[ "$#" -ne 0 ]]; then
   for f in $@; do
      root_dir=src
      if [[ $f == *"test/"* ]]; then
        root_dir=test
      fi
      $SCRIPTPATH/cpplint.py --root=$root_dir --linelength=80 --filter=-build/c++11,-legal/copyright $f
   done
   exit 0
else
  help
fi

# run cpplint for all staged source files in dir src/ and include/
files=$($git_cmd | grep "^\(src\|include\)/.*" | grep "\(\.h\|\.cc\)$")
num_files=$(echo "$files" | sed '/^$/d' | wc -l)
if [ $num_files != "0" ]; then
  echo "$files" | xargs $SCRIPTPATH/cpplint.py --root=src --linelength=80 --filter=-build/c++11,-legal/copyright
else
  echo "Nothing to be checked for directory src"
fi

# run cpplint for all staged source files in dir test
files=$($git_cmd | grep "^test/.*" | grep "\(\.h\|\.cc\)$")
num_files=$(echo "$files" | sed '/^$/d' | wc -l)
if [ $num_files != "0" ]; then
  echo "$files" | xargs $SCRIPTPATH/cpplint.py --root=test --linelength=80 --filter=-build/c++11,-legal/copyright
else
  echo "Nothing to be checked for directory test"
fi

exit 0
