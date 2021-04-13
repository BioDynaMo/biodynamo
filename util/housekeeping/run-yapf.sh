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
# Runs yapf in the given directory
# Arguments:
#   $1 - Path to the source tree (use absolute path!)
#   $2 - Mode (0, 1, or 2)
#        0: raise an error if there were changes
#        1: apply fixes
#        2: display differences
#   $ARGN - Files to run YAPF on
#
# This scirpt was designed to be similar to run-clang-format and run-clang-tidy.
#

# print header 
echo ""
echo "#################### BioDynaMo's YAPF style check ####################"
echo ""

# Test if yapf is present in python environment
if [[ -z "$(python3.9 -m pip list | grep yapf)" ]]; then
  echo "Error: YAPF is not installed in your default python interpreter."
  echo "Make sure that you have the correct environment enabled."
  echo "YAPF can be installed via pip: 'python3.9 -m pip install yapf'."
  echo "Alternatively, execute './prerequisites.sh' in source directory."
  exit 1
fi

 # Test if script receives the correct input of arguments
if [ "$#" -lt 3 ]; then
  echo "Error: $0 requires at least 3 arguments:"
  echo "Usage: $0 <path> <mode> <list_of_files>"
  echo "Possibly the scirpt received no files, e.g. because you wanted to run"
  echo "YAPF on staged files but there are none."
  echo ""
  echo "Received command line arguments: (<path> <mode> <list_of_files>)"
  echo $@
  exit 1
fi

# get command line arguments
SOURCE_DIR=$1
shift
MODE=$1
shift

# yapf will only find its configuration if we are in
# the source tree or in a path relative to the source tree
pushd $SOURCE_DIR >/dev/null

# Execute YAPF with different modes
if [ "$MODE" == "0" ]; then
  # Test if YAPF would do some changes
  if [[ $(yapf -d -p $@) ]]; then
    echo "Error: YAPF found format errors, please fix them."
    echo "The erros can be listed with 'make show-yapf*' or corrected with"
    echo "'make yapf*'. Alternatively, run 'run-yapf.sh' with mode '1' or '2'."
    echo ""
    echo "######################################################################"
    popd >/dev/null
    exit 1
  else
    echo "Check files:"
    for f in $@; do
      echo $f
    done
    echo ""
    echo "YAPF: All (checked) python files are formatted correctly."
  fi
# Apply changes inplace, see YAPF documentation for usage flags
elif [ "$MODE" == "1" ]; then
  yapf -i -p -vv $@
# List suggested changes, see YAPF documentation for usage flags
elif [ "$MODE" == "2" ]; then
  yapf -d -vv $@
else
  echo "Error: mode arg. must be 0, 1, or 2 for run-yapf.sh"
fi

# print closing statement
echo ""
echo "######################################################################"

popd >/dev/null