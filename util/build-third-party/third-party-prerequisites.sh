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
  if [ $BDM_OS = "centos-7" ]; then

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
    sudo yum install -y git binutils \
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

    # libroadrunner
    sudo yum install -y llvm-toolset-6.0-llvm-devel llvm-toolset-6.0-llvm-static
    sudo yum install -y ncurses-devel
    sudo yum install -y libxml2-devel
    sudo yum install -y bzip2 bzip2-devel
    sudo yum install -y zlib zlib-devel

    # Install dependencies to install Python with PyEnv
    sudo yum install -y @development zlib-devel bzip2 bzip2-devel readline-devel sqlite \
      sqlite-devel openssl-devel xz xz-devel libffi-devel findutils

    export LLVM_CONFIG="/opt/rh/llvm-toolset-6.0/root/usr/bin/llvm-config"
    set +e
    . scl_source enable devtoolset-7
    . scl_source enable llvm-toolset-6.0
    set -e
    CC=gcc
    CXX=g++
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
    sudo apt-get -y install git dpkg-dev g++ gcc binutils libx11-dev libxpm-dev \
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

    # Install dependencies to install Python with PyEnv
    sudo apt-get install -y libssl-dev zlib1g-dev libbz2-dev \
      libreadline-dev libsqlite3-dev wget curl llvm \
      xz-utils tk-dev libffi-dev liblzma-dev python-openssl git

    sudo apt install -y libxt-dev freeglut3-dev

    # libroadrunner
    sudo apt-get install -y llvm-6.0 llvm-6.0-dev llvm-6.0-runtime
    sudo apt-get install -y libbz2-1.0 libbz2-dev zlibc libxml2-dev libz-dev
    sudo apt-get install -y libncurses5-dev

    export LLVM_CONFIG="/usr/bin/llvm-config-6.0"
  fi
  # update cmake to build ROOT
  URL="https://cmake.org/files/v3.15/cmake-3.15.3-Linux-x86_64.tar.gz"
  DownloadTarAndExtract $URL $WORKING_DIR/cmake-3.15.3 1
  export PATH=$WORKING_DIR/cmake-3.15.3/bin:$PATH

  # Install pyenv and python 3.6.9
  curl https://pyenv.run | bash
  export PATH="$HOME/.pyenv/bin:$PATH"
  eval "$(pyenv init -)"
  env PYTHON_CONFIGURE_OPTS="--enable-shared" pyenv install 3.6.9
  pyenv shell 3.6.9

else
  brew install llvm@6
  brew install swig
  brew install git

  export CXX=/usr/local/opt/llvm@6/bin/clang++
  export CC=/usr/local/opt/llvm@6/bin/clang

  export LLVM_CONFIG="/usr/bin/llvm-config-6"

  xcode-select --install || true
  brew install cmake

  # Install pyenv and python 3.6.9
  curl https://pyenv.run | bash
  export PATH="$HOME/.pyenv/bin:$PATH"
  eval "$(pyenv init -)"
  env PYTHON_CONFIGURE_OPTS="--enable-shared" pyenv install 3.6.9
  pyenv shell 3.6.9
fi
