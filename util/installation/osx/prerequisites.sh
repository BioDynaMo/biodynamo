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

if [ ! -x "/usr/local/bin/brew" ]; then
   echo "First install the Homebrew macOS package manager from https://brew.sh"
   exit 1
fi

if [ ! -x "/usr/bin/git" ]; then
   echo "First install Xcode (from the App Store) and the command line tools"
   echo "using the command \"xcode-select --install\"."
   exit 1
fi

brew update
brew style
brew update-reset

# Install and upgrade required packages
brew install \
  $(cat $BDM_PROJECT_DIR/util/installation/osx/package_list_required) || true
brew upgrade cmake || true

# Install Python 3.8.0 environment
PYVERS=3.8.0
export PYENV_ROOT=/usr/local/opt/.pyenv
eval "$(pyenv init -)"
if [ ! -f  "$PYENV_ROOT/versions/$PYVERS/lib/libpython3.8.dylib" ]; then
   echo "Python $PYVERS was not found. Installing now..."
   /usr/bin/env PYTHON_CONFIGURE_OPTS="--enable-shared" pyenv install -f $PYVERS
fi
pyenv shell $PYVERS

# Install the optional packages
if [ $1 == "all" ]; then
    PIP_PACKAGES="nbformat jupyter metakernel"
    python -m pip install --user $PIP_PACKAGES
    brew install \
      $(cat $BDM_PROJECT_DIR/util/installation/osx/package_list_extra) || true
fi

exit 0
