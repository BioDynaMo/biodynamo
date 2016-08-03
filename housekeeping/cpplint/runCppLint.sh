#!/bin/bash

function help {
  echo "runCppLint.py <mode>
  Executes cpplint on a filtered subset of *.h and *.cc files in directory include, source and test
  <mode> defines the filter condition: options
     staged: only files that are staged in git are checked
     master: all files that have been changed compared to the local master HEAD are checked"
  exit
}

git_cmd=""
if [ "$1" == "staged" ]; then
  git_cmd="git --no-pager diff --name-only --cached"
elif [ "$1" == "master" ]; then
  git_cmd="git --no-pager diff --name-only FETCH_HEAD $(git merge-base FETCH_HEAD master)"
else
  help
fi

# run cpplint for all staged source files in dir src/ and include/
files=$($git_cmd | grep -P "^((src/)|(include/))" | grep "\(\.h\)\|\(\.cc\)$")
num_files=$(echo "$files" | sed '/^$/d' | wc -l)
if [ $num_files != "0" ]; then
  echo "$files" | xargs housekeeping/cpplint/cpplint.py --root=include --linelength=120 --filter=-build/c++11,-legal/copyright
else
  echo "Nothing to be checked for directory src and include"
fi

# run cpplint for all staged source files in dir test
files=$($git_cmd | grep -P "^test/" | grep "\(\.h\)\|\(\.cc\)$")
num_files=$(echo "$files" | sed '/^$/d' | wc -l)
if [ $num_files != "0" ]; then
  echo "$files" | xargs housekeeping/cpplint/cpplint.py --linelength=120 --filter=-build/c++11,-legal/copyright
else
  echo "Nothing to be checked for directory test"
fi