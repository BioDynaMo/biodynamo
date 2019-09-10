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

# Installs the required packages to build the third_party dependencies
# NB: This script must be sourced!

# Check if we already sourced this script before
if [ -z ${third_party_prerequisites_sourced} ]; then
  export third_party_prerequisites_sourced=1
else
  return 0
fi

# Script path
SCRIPTPATH="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# source the utils so to get DetectOS in place
. ${SCRIPTPATH}/../installation/common/util.sh

if [ `uname` = "Linux" ]; then
  BDM_OS=$(DetectOs)
  if [ $BDM_OS = "centos-7.6.1810" ]; then

    # Add custom repository for llvm-toolset-6.0
    sudo bash -c 'cat << EOF  > /etc/yum.repos.d/springdale-7-SCL.repo
[SCL-core]
name=Springdale SCL Base 7.6 - x86_64
mirrorlist=http://springdale.princeton.edu/data/springdale/SCL/7.6/x86_64/mirrorlist
#baseurl=http://springdale.princeton.edu/data/springdale/SCL/7.6/x86_64
gpgcheck=1
gpgkey=http://springdale.math.ias.edu/data/puias/7.6/x86_64/os/RPM-GPG-KEY-puias
EOF'
    sudo yum update -y

    #  root required packages
    sudo yum install -y git cmake cmake3 binutils \
      libX11-devel libXpm-devel libXft-devel libXext-devel
    #  root optional packages
    sudo yum install -y gcc-gfortran openssl-devel pcre-devel \
      mesa-libGL-devel mesa-libGLU-devel glew-devel ftgl-devel mysql-devel \
      fftw-devel cfitsio-devel graphviz-devel \
      avahi-compat-libdns_sd-devel libldap-dev python-devel \
      libxml2-devel gsl-static || true
    # paraview
    ## issues with mpich and valgrind:
    ## https://github.com/flow123d/flow123d/issues/806
    sudo yum install -y openmpi3-devel || true
    . /etc/profile.d/modules.sh
    module load mpi

    sudo yum install -y libXt-devel freeglut3-devel

    sudo yum install -y centos-release-scl epel-release
    sudo yum -y install https://centos7.iuscommunity.org/ius-release.rpm || true
    sudo yum install -y devtoolset-7-gcc*
    export LD_LIBRARY_PATH=/opt/rh/devtoolset-7/root/usr/lib64:/opt/rh/devtoolset-7/root/usr/lib:/opt/rh/devtoolset-7/root/usr/lib64/dyninst:/opt/rh/devtoolset-7/root/usr/lib/dyninst:$LD_LIBRARY_PATH
    export PATH=/opt/rh/devtoolset-7/root/usr/bin:$PATH

    # libroadrunner
    sudo yum install -y llvm-toolset-6.0-llvm-devel llvm-toolset-6.0-llvm-static
    sudo yum install -y ncurses-devel
    sudo yum install -y libxml2-devel
    sudo yum install -y bzip2 bzip2-devel
    sudo yum install -y zlib zlib-devel

    export LLVM_CONFIG="/opt/rh/llvm-toolset-6.0/root/usr/bin/llvm-config"
    set +e
    . scl_source enable llvm-toolset-6.0
    set -e

    # Set cmake3 as the default cmake
    sudo alternatives --install /usr/local/bin/cmake cmake /usr/bin/cmake3 20 \
    --slave /usr/local/bin/ctest ctest /usr/bin/ctest3 \
    --slave /usr/local/bin/cpack cpack /usr/bin/cpack3 \
    --slave /usr/local/bin/ccmake ccmake /usr/bin/ccmake3 \
    --family cmake

  else
    if [ $BDM_OS = "travis-linux" ]; then
      sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
      sudo apt-get update
      sudo apt-get -y install gcc-5 g++-5
      CC=gcc-5
      CXX=g++-5
    else
      sudo apt update
      sudo apt-get -y install gcc g++
      CC=gcc
      CXX=g++
    fi
    # only for ubuntu image
    sudo apt-get -y install wget git make
    #  root required packages
    sudo apt-get -y install git dpkg-dev cmake g++ gcc binutils libx11-dev libxpm-dev \
      libxft-dev libxext-dev
    #  root optional packages
    sudo apt-get -y install gfortran libssl-dev libpcre3-dev \
      xlibmesa-glu-dev libglew1.5-dev libftgl-dev \
      libmysqlclient-dev libfftw3-dev libcfitsio-dev \
      graphviz-dev libavahi-compat-libdnssd-dev \
      libldap2-dev python-dev libxml2-dev libkrb5-dev \
      libgsl0-dev libqt4-dev || true
    # paraview
    sudo apt-get -y install libopenmpi-dev || true

    sudo apt install -y libxt-dev freeglut3-dev

    # libroadrunner
    sudo apt-get install -y llvm-6.0 llvm-6.0-dev llvm-6.0-runtime
    sudo apt-get install -y libbz2-1.0 libbz2-dev zlibc libxml2-dev libz-dev
    sudo apt-get install -y libncurses5-dev

    export LLVM_CONFIG="/usr/bin/llvm-config-6.0"

  fi

else
  brew install llvm@6 || true
  brew install swig || true
  brew install git || true

  export CXX=/usr/local/opt/llvm@6/bin/clang++
  export CC=/usr/local/opt/llvm@6/bin/clang

  export LLVM_CONFIG="/usr/local/Cellar/llvm@6/6.0.1_1/bin/llvm-config"

  xcode-select --install || true
  brew install cmake || true
fi
