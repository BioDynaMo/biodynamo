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

#This script installs the required packages for ubuntu 16.04
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

# Required to add Kitware ppa below
sudo apt-get update
sudo apt-get install apt-transport-https

# Add ppa for newer CMake version
wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | sudo apt-key add -
REPO="deb https://apt.kitware.com/ubuntu/ `lsb_release -cs` main"
sudo apt-add-repository "$REPO"

# Update
sudo apt-get update

# Install required packages
sudo apt-get install -y \
  $(cat $BDM_PROJECT_DIR/util/installation/ubuntu-16.04/package_list_required)

if [ -n "${PYENV_ROOT}" ]; then
  unset PYENV_ROOT
fi

# Install dependencies to install Python with PyEnv
sudo apt-get install -y \
  $(cat $BDM_PROJECT_DIR/util/installation/ubuntu-16.04/package_list_pyenv)

# If PyEnv is not installed, install it
if [ ! -f "$HOME/.pyenv/bin/pyenv" ]; then
  echo "PyEnv was not found. Installing now..."
  curl https://pyenv.run | bash
fi
export PATH="$HOME/.pyenv/bin:$PATH"
eval "$(pyenv init -)"

PYVERS=3.6.9

# If Python $PYVERS is not installed, install it
if [ ! -f  "$HOME/.pyenv/versions/$PYVERS/lib/libpython3.so" ]; then
  echo "Python $PYVERS was not found. Installing now..."
  env PYTHON_CONFIGURE_OPTS="--enable-shared" pyenv install -f $PYVERS
fi
pyenv shell $PYVERS

# Install optional packages
if [ $1 == "all" ]; then
  # this updates pip, but installs the updated version in $HOME/.local/bin
  PIP_PACKAGES="nbformat jupyter metakernel"
  pip install --user $PIP_PACKAGES

  sudo apt-get install -y \
    $(cat $BDM_PROJECT_DIR/util/installation/ubuntu-16.04/package_list_extra)
fi
