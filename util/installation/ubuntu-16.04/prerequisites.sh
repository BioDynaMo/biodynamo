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

# Add ppa for newer CMake version
wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | sudo apt-key add -
REPO="deb https://apt.kitware.com/ubuntu/ `lsb_release -cs` main"
sudo apt-add-repository "$REPO"

# Update
sudo apt-get update

# Install required packages
sudo apt-get install -y wget curl cmake make gcc g++ \
  libopenmpi-dev libomp-dev libnuma-dev freeglut3-dev \
  libpthread-stubs0-dev zlib1g-dev libbz2-dev

# On Travis CI pyenv is already installed, so we need to unset the following
unset PYENV_ROOT

# Install dependencies to install Python with PyEnv
sudo apt-get install -y libssl-dev zlib1g-dev libbz2-dev libreadline-dev \
  libsqlite3-dev xz-utils tk-dev libffi-dev liblzma-dev python-openssl

curl https://pyenv.run | bash
export PATH="$HOME/.pyenv/bin:$PATH"
eval "$(pyenv init -)"
env PYTHON_CONFIGURE_OPTS="--enable-shared" pyenv install 3.6.9
pyenv shell 3.6.9

# Install optional packages
if [ $1 == "all" ]; then
  # this updates pip, but installs the updated version in $HOME/.local/bin
  PIP_PACKAGES="nbformat jupyter metakernel"
  pip install --user $PIP_PACKAGES

  sudo apt-get install -y valgrind \
    clang-3.9 clang-format-3.9 clang-tidy-3.9 \
    doxygen graphviz lcov gcovr \
    llvm-6.0 llvm-6.0-dev llvm-6.0-runtime libxml2-dev
fi
