#!/bin/bash
# -----------------------------------------------------------------------------
#
# Copyright (C) The BioDynaMo Project.
# All Rights Reserved.
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
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
#
# Runs clang-tidy in the given directory
# Arguments:
#   $1 - Path to the clang-tidy binary
#   $2 - Path to the clang-tidy header helper source file
#   $3 - Path to the compile_commands.json to use
#   $4 - Mode (0, 1, or 2)
#        0: raise an error if there were warnings
#        1: apply fixes
#        2: display warnings
#   $ARGN - Files that should be tested

# Unlike clang-format, clang-tidy cannot be executed on a header file alone.
# It requires a translation unit TU (*.cc) = source file.
# However, if we want to test a list of header files we do not know which source
# file to choose.
# Therefore, we generate a source file (parameter $3) which includes them

CLANG_TIDY=$1
shift
TIDY_HEADER_HELPER=$1
shift
COMPILE_COMMANDS=$1
shift
MODE=$1
shift

if [ "$#" == "0" ]; then
    echo "Warning: No files to process."
    echo -e "This may happen because the repository has no origin/master branch set."
    echo "Try to add it using 'git remote add' or run 'make tidy-all' instead."
    exit 0;
fi

echo "Process files: "
for f in $@; do
    echo $f
done

# build line filter string
LINE_FILTER="["
for f in $@; do
    LINE_FILTER=$LINE_FILTER"{\"name\":\"${f}\"},"
done
LINE_FILTER=${LINE_FILTER%?}
LINE_FILTER=$LINE_FILTER"]"

# extract list of source files from $@
SOURCES=" "
for f in $@; do
  if [ "$(echo $f | grep .cc$ | wc -l)" == "1" ]; then
    SOURCES=$SOURCES" "$f
  fi
done

HEADERS=" "
for f in $@; do
  if [ "$(echo $f | grep .h$ | wc -l)" == "1" ]; then
    HEADERS=$HEADERS" "$f
  fi
done

# fix issue with finding omp.h on Travis-CI build on linux with g++
TRAVIS_LINUX_EXTRA_ARGB=""
if [ "$TRAVIS_OS_NAME" = "linux" ]; then
  TRAVIS_LINUX_EXTRA_ARGB="-extra-arg-before=-I/tmp/bdm_omp/"
fi

function EchoError {
  echo "Error: clang-tidy suggested changes, please fix them!"
  echo "       Before running one of the clang-tidy* targets to fix them automatically,"
  echo "       it is recommended to have a look at the warnings first."
  echo "       Use the corresponding 'make show-clang-tidy*' target"
}

if [ "$MODE" == "1" ]; then
  # fix errors one source file at a time
  for f in $SOURCES; do
    $CLANG_TIDY $TRAVIS_LINUX_EXTRA_ARGB -line-filter=$LINE_FILTER -p $COMPILE_COMMANDS -fix $f
  done
  # fix errors one header file at a time
  for f in $HEADERS; do
    echo "" > $TIDY_HEADER_HELPER
    echo "#include \"${f}\"" >> $TIDY_HEADER_HELPER
    $CLANG_TIDY $TRAVIS_LINUX_EXTRA_ARGB -line-filter=$LINE_FILTER -p $COMPILE_COMMANDS -fix $TIDY_HEADER_HELPER
  done
elif [ "$MODE" == "2" ]; then
  # process source files one at a time
  for f in $SOURCES; do
    echo "Start processing: "$f
    $CLANG_TIDY $TRAVIS_LINUX_EXTRA_ARGB -line-filter=$LINE_FILTER -p $COMPILE_COMMANDS $f
  done
  # process header files one at a time
  for f in $HEADERS; do
    echo "Start processing: "$f
    echo "" > $TIDY_HEADER_HELPER
    echo "#include \"${f}\"" >> $TIDY_HEADER_HELPER
    $CLANG_TIDY $TRAVIS_LINUX_EXTRA_ARGB -line-filter=$LINE_FILTER -p $COMPILE_COMMANDS $TIDY_HEADER_HELPER
  done
else
  # process source files one at a time
  TMP_FILE="/tmp/bdmformat_"$RANDOM
  for f in $SOURCES; do
    echo "Start processing: "$f
    echo "" > $TMP_FILE
    $CLANG_TIDY $TRAVIS_LINUX_EXTRA_ARGB -line-filter=$LINE_FILTER -export-fixes=$TMP_FILE -p $COMPILE_COMMANDS $f >/dev/null 2>/dev/null
    NUM_CORRECTIONS=$(cat $TMP_FILE | wc -l)
    rm -f $TMP_FILE
    if [ "$NUM_CORRECTIONS" -gt "1" ]; then
      EchoError
      exit 1
    fi
  done
  # process header files one at a time
  for f in $HEADERS; do
    echo "Start processing: "$f
    echo "" > $TMP_FILE
    echo "" > $TIDY_HEADER_HELPER
    echo "#include \"${f}\"" >> $TIDY_HEADER_HELPER
    $CLANG_TIDY $TRAVIS_LINUX_EXTRA_ARGB -line-filter=$LINE_FILTER -export-fixes=$TMP_FILE -p $COMPILE_COMMANDS $TIDY_HEADER_HELPER >/dev/null 2>/dev/null
    NUM_CORRECTIONS=$(cat $TMP_FILE | wc -l)
    rm -f $TMP_FILE
    if [ "$NUM_CORRECTIONS" -gt "1" ]; then
      EchoError
      exit 1
    fi
  done
fi
