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

if [ `uname` = "Linux" ]; then
  BDM_OS=$(DetectOs)
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

  sudo apt install -y libxt-dev freeglut3-dev

  # libzmqpp
  sudo apt-get install --quiet libzmq5 libzmq3-dev

  # update cmake
  URL="https://cmake.org/files/v3.6/cmake-3.6.3-Linux-x86_64.tar.gz"
  DownloadTarAndExtract $URL $WORKING_DIR/cmake-3.6.3 1
  export PATH=$WORKING_DIR/cmake-3.6.3/bin:$PATH
else
  brew install llvm
  CC=/usr/local/opt/llvm/bin/clang
  CXX=/usr/local/opt/llvm/bin/clang++
  xcode-select --install || true
  brew install cmake
fi
