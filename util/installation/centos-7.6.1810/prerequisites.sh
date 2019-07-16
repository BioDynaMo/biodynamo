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

# This script installs the required packages for ubuntu 16.04
if [[ $# -ne 1 ]]; then
  echo "ERROR: Wrong number of arguments.
Description:
  This script installs the prerequisites of BioDynaMo, but not BioDynaMo
  itself. Script install.sh installs both prerequisites and BioDynaMo.
No Arguments"
  exit 1
fi

sudo -v
# add repository for clang-3.9
sudo yum update -y

# install packages
sudo yum -y install centos-release-scl epel-release
sudo yum -y install https://centos7.iuscommunity.org/ius-release.rpm

sudo yum -y install cmake3 libXt-devel libXext-devel \
devtoolset-7-gcc* numactl-devel \
tbb-devel openmpi3-devel freeglut-devel \
rh-python36 \
git

# Install optional packages
if [ $1 == "all" ]; then
    pip install --user mkdocs mkdocs-material
    sudo yum -y install lcov gcovr llvm-toolset-7 llvm-toolset-7-clang-tools-extra doxygen graphviz valgrind
fi

# Set up cmake alias such to be able to use it
sudo alternatives --install /usr/local/bin/cmake cmake /usr/bin/cmake3 20 \
--slave /usr/local/bin/ctest ctest /usr/bin/ctest3 \
--slave /usr/local/bin/cpack cpack /usr/bin/cpack3 \
--slave /usr/local/bin/ccmake ccmake /usr/bin/ccmake3 \
--family cmake
