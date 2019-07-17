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

# Install required packages
sudo apt-get install -y cmake make gcc g++ \
libopenmpi-dev libomp-dev libnuma-dev libtbb-dev freeglut3-dev libpthread-stubs0-dev \
git \
python3 python3-dev python3-pip

# Install optional packages
if [ $1 == "all" ]; then
    pip3 install --user mkdocs mkdocs-material
    sudo apt-get install -y valgrind \
    clang-3.9 clang-format-3.9 clang-tidy-3.9 \
    doxygen graphviz \
    lcov gcovr
fi
