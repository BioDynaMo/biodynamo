#!/bin/bash
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
# Runs clang-format in the given directory
# Arguments:
#   $1 - Path to the source tree
#   $2 - Path to the clang-format binary
#   $3 - Mode (0, 1, or 2)
#        0: raise an error if there were changes
#        1: apply fixes
#        2: display differences
#   $ARGN - Files to run clang format on
#
SOURCE_DIR=$1
shift
CLANG_FORMAT=$1
shift
MODE=$1
shift

if [ "$#" == "0" ]; then
  echo "Warning: No files to process"
  exit 0;
fi

echo "Process files: "
for f in $@; do
    echo $f
done
echo ""

# clang-format will only find its configuration if we are in
# the source tree or in a path relative to the source tree
pushd $SOURCE_DIR >/dev/null

if [ "$MODE" == "1" ]; then
  $CLANG_FORMAT -i $@
elif [ "$MODE" == "2" ]; then
  RAND=$RANDOM
  for f in $@; do
      $CLANG_FORMAT $f >/tmp/$RAND
      diff -c $f /tmp/$RAND
  done
else
  NUM_CORRECTIONS=`$CLANG_FORMAT -output-replacements-xml $@ | grep offset | wc -l`

  if [ "$NUM_CORRECTIONS" -gt "0" ]; then
    echo "Error: clang-format suggested changes, please fix them!"
    popd >/dev/null
    exit 1
  fi
fi
popd >/dev/null
