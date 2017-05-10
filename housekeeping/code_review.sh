#!/bin/bash
# This script assists the code review process. It checks code formatting and
# style violations for the list of files that have been changed compared to
# the current master branch. Furthermore, it creates the doxygen documentation
# and coverage report and opens them in the browser


function CheckFormatting {
  if [[ "$1" == *\.h || "$1" == *\.cc ]]; then
    echo "Formatting errors for file: $@"
    clang-format-3.7 $1 | diff $1 -
    echo
  fi
}
# export function so it can be used by xargs
export -f CheckFormatting

current_dir=$(pwd)
if [[ "$current_dir" == *biodynamo\/build ]]; then
  cd ..
  # update master
  echo
  echo "-----------------------------------------------------------------------"
  echo "Update master"
  echo "-----------------------------------------------------------------------"
  git fetch origin master

  echo
  echo "-----------------------------------------------------------------------"
  echo "run cppLint for all files that changed compared to master"
  echo "-----------------------------------------------------------------------"
  git diff --name-only origin/master | xargs housekeeping/cpplint/runCppLint.sh

  echo
  echo "-----------------------------------------------------------------------"
  echo "run clang-format for all files that changed compared to master"
  echo "-----------------------------------------------------------------------"
  git diff --name-only origin/master | xargs -L 1 bash -c 'CheckFormatting "$1"' _

  # go back to original direcory
  cd $current_dir

  echo
  echo "-----------------------------------------------------------------------"
  echo "generate and open doxygen"
  echo "-----------------------------------------------------------------------"
  make doc >/dev/null
  xdg-open doc/html/index.html

  echo
  echo "-----------------------------------------------------------------------"
  echo "generate and open coverage report"
  echo "-----------------------------------------------------------------------"
  mkdir coverage ; cd coverage
  pwd
  cmake -Dcoverage=on ../.. >/dev/null
  make -j5 >/dev/null
  make coverage >/dev/null
  xdg-open coverage/index.html

  # go back to original direcory
  cd $current_dir
else
    echo "This script must be run from biodynamo/build"
fi
