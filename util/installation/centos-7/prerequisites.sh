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
Arguments:
    <install_type>  all/required. If all is specified, then this script
                    will install all the prerequisites."
  exit 1
fi

sudo -v
# add repository for clang-3.9
sudo yum update -y

# install packages
sudo yum -y install centos-release-scl epel-release
sudo yum -y install https://centos7.iuscommunity.org/ius-release.rpm

sudo yum -y install wget cmake3 libXt-devel libXext-devel \
devtoolset-7-gcc* numactl-devel \
tbb-devel openmpi3-devel freeglut-devel \
rh-python36 python python-pip git

# Install optional packages
if [ $1 == "all" ]; then
    pip install --user mkdocs mkdocs-material
    sudo yum -y install lcov gcovr llvm-toolset-7 llvm-toolset-7-clang-tools-extra doxygen graphviz valgrind
    # SBML integration
    sudo bash -c 'cat << EOF  > /etc/yum.repos.d/springdale-7-SCL.repo
[SCL-core]
name=Springdale SCL Base 7.6 - x86_64
mirrorlist=http://springdale.princeton.edu/data/springdale/SCL/7.6/x86_64/mirrorlist
#baseurl=http://springdale.princeton.edu/data/springdale/SCL/7.6/x86_64
gpgcheck=1
gpgkey=http://springdale.math.ias.edu/data/puias/7.6/x86_64/os/RPM-GPG-KEY-puias
EOF'
    sudo yum update -y
    sudo yum install -y llvm-toolset-6.0-llvm-devel llvm-toolset-6.0-llvm-static
    sudo yum install -y libxml2-devel
fi

# Set up cmake alias such to be able to use it
# FIXME: this is will basically change permanently the system of the user
sudo alternatives --install /usr/local/bin/cmake cmake /usr/bin/cmake3 20 \
--slave /usr/local/bin/ctest ctest /usr/bin/ctest3 \
--slave /usr/local/bin/cpack cpack /usr/bin/cpack3 \
--slave /usr/local/bin/ccmake ccmake /usr/bin/ccmake3 \
--family cmake
