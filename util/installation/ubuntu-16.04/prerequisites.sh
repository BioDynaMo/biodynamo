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

# Update
sudo apt-get update

# Install required packages
sudo apt-get install -y wget cmake make gcc g++ \
libopenmpi-dev libomp-dev libnuma-dev libtbb-dev freeglut3-dev \
libpthread-stubs0-dev python-pip python3-pip zlib1g-dev libbz2-dev

# Install optional packages
if [ $1 == "all" ]; then
    # this updates pip, but installs the updated version in $HOME/.local/bin
    pip install --upgrade pip
    PIP_PACKAGES="jupyter metakernel"
    if [ -f "$HOME/.local/bin/pip2" ]; then
      $HOME/.local/bin/pip2 install --user $PIP_PACKAGES
    else
      echo "WARNING: $HOME/.local/bin/pip2 not found.
The following pip packages will not be installed: $PIP_PACKAGES"
    fi

    sudo apt-get install -y valgrind \
      clang-3.9 clang-format-3.9 clang-tidy-3.9 \
      doxygen graphviz lcov gcovr \
      llvm-6.0 llvm-6.0-dev llvm-6.0-runtime libxml2-dev
fi
