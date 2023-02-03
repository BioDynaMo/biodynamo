#!/bin/bash
# -----------------------------------------------------------------------------
#
# Copyright (C) 2021 CERN & University of Surrey for the benefit of the
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

#This script installs the required packages
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
CODENAME=$(grep -oP '(?<=^UBUNTU_CODENAME=).+' /etc/os-release | tr -d '"')
REPO="deb https://apt.kitware.com/ubuntu/ ${CODENAME} main"
sudo apt-add-repository "$REPO"

# Update
sudo apt-get update

# Install required packages
sudo apt-get install -y \
  $(cat $BDM_PROJECT_DIR/util/installation/ubuntu-18.04/package_list_required)

if [ -n "${PYENV_ROOT}" ]; then
  unset PYENV_ROOT
fi

# If PyEnv is not installed, install it
if [ ! -f "$HOME/.pyenv/bin/pyenv" ]; then
  echo "PyEnv was not found. Installing now..."
  curl https://pyenv.run | bash
fi

export PYENV_ROOT="$HOME/.pyenv"
export PATH="$PYENV_ROOT/bin:$PATH"
eval "$(pyenv init --path)"
eval "$(pyenv init -)"
pyenv update

PYVERS=3.9.1

# If Python $PYVERS is not installed, install it
if [ ! -f  "$HOME/.pyenv/versions/$PYVERS/lib/libpython3.so" ]; then
  echo "Python $PYVERS was not found. Installing now..."
  /usr/bin/env PYTHON_CONFIGURE_OPTS="--enable-shared" pyenv install -f $PYVERS
fi
pyenv shell $PYVERS

# Install optional packages
if [ $1 == "all" ]; then
  # Don't install --user: the packages should end up in the PYENV_ROOT directory
  python -m pip install -r $BDM_PROJECT_DIR/util/installation/ubuntu-18.04/pip_packages.txt

  sudo apt-get install -y \
    $(cat $BDM_PROJECT_DIR/util/installation/ubuntu-18.04/package_list_extra)
fi

exit 0
