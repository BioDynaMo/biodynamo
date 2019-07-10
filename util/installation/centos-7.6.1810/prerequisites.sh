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

# CentOs specifics. This is done manually instead of using
# the standard commands:
# scl enable devtoolset-7 bash
# scl enable rh-python36 bash
#if versionLessThan "$(python3 --version)" 'Python 3.2.0'; then
#  export PATH=/opt/rh/rh-python36/root/bin:/opt/rh/llvm-toolset-7/root/usr/bin/:$PATH
#fi

# Export path needed for detecting correctly g++ and gcc
#export PATH=/opt/rh/devtoolset-7/root/usr/bin:$PATH
#export LD_LIBRARY_PATH=/opt/rh/devtoolset-7/root/usr/lib:/opt/rh/devtoolset-7/root/usr/lib/dyninst:/opt/rh/devtoolset-7/root/usr/lib64:/opt/rh/devtoolset-7/root/usr/lib64/dyninst:$LD_LIBRARY_PATH

sudo -v
# add repository for clang-3.9
sudo yum update -y

# install packages
sudo yum -y install centos-release-scl epel-release
sudo yum -y install https://centos7.iuscommunity.org/ius-release.rpm

sudo yum -y install cmake devtoolset-7-gcc* numactl-devel \
tbb-devel openmpi3-devel freeglut-devel \
python35u python35u-devel python35u-pip \
git

# Install optional packages
if [ $1 == "all" ]; then
    pip install --user mkdocs mkdocs-material
    sudo yum -y install lcov gcovr llvm-toolset-7 llvm-toolset-7-clang-tools-extra doxygen graphviz
fi


# activate mpi
#. /etc/profile.d/modules.sh
#module load mpi

