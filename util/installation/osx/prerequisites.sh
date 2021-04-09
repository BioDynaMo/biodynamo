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

if [[ $# -ne 1 ]]; then
  echo "ERROR: Wrong number of arguments.
Description:
    This script installs the prerequisites of BioDynaMo, but not BioDynaMo
    itself. Script install.sh installs both prerequisites and BioDynaMo.
Arguments:
    <install_type>  all/required. If all is specified, then this script
                    will install all the prerequisites."
  exit 1
fi

BDM_PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../../.."

if [ `arch` == "i386" -a ! -x "/usr/local/bin/brew" ]; then
   echo "First install the Homebrew macOS package manager from https://brew.sh"
   exit 1
elif [ `arch` == "arm64" -a ! -x "/opt/homebrew/bin/brew" ]; then
   echo "First install the Homebrew macOS package manager from https://brew.sh"
   exit 1
fi

if [ ! -x "/usr/bin/git" ]; then
   echo "First install Xcode (from the App Store) and the command line tools"
   echo "using the command \"xcode-select --install\"."
   exit 1
fi

# Install and upgrade required packages
brew install \
  $(cat $BDM_PROJECT_DIR/util/installation/osx/package_list_required) || true

# Install the optional packages
if [ $1 == "all" ]; then
    PIP_PACKAGES="nbformat jupyter metakernel jupyterlab yapf"
    # Don't install --user: the packages should end up in the PYENV_ROOT directory
    python3.9 -m pip install $PIP_PACKAGES
    brew install \
      $(cat $BDM_PROJECT_DIR/util/installation/osx/package_list_extra) || true
fi

exit 0
