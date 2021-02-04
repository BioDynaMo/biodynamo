#!/bin/bash
# -----------------------------------------------------------------------------
#
# Copyright (C) 2021 CERN & Newcastle University for the benefit of the
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

# Installs the required packages to build the third_party dependencies

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
. "${SCRIPT_DIR}/env.sh"

set -e -x

cd "$WORKING_DIR"

if [ -z "$SKIP_PACKAGE_MAN" ]; then
  if [ $(uname) = "Linux" ]; then
    if [ $BDM_OS = "centos-7" ]; then
      sudo yum update -y || true

      # paraview
      ## issues with mpich and valgrind:
      ## https://github.com/flow123d/flow123d/issues/806
      sudo yum install -y openmpi3-devel || true
      sudo yum install -y libXt-devel freeglut3-devel

      sudo yum install -y centos-release-scl epel-release
      sudo yum install -y https://centos7.iuscommunity.org/ius-release.rpm || true
      sudo yum install -y devtoolset-7-gcc*
      sudo yum install -y ninja-build
      sudo yum install -y rsync

      # OpenGL packages
      sudo yum install -y mesa-libGL-devel mesa-libGLU-devel glew-devel ftgl-devel

      # Install dependencies to install Python with PyEnv
      sudo yum install -y @development zlib-devel bzip2 bzip2-devel readline-devel sqlite \
        sqlite-devel openssl-devel xz xz-devel libffi-devel findutils

      if [ "$PV_FLAVOR" = "nvidia-headless" ]; then
        sudo yum install -y mesa-libEGL-devel libglvnd libglvnd-egl libglvnd-opengl
      fi
    else
      sudo apt update
      sudo apt-get -y install gcc g++
      sudo apt-get -y install rsync wget git make
      # paraview
      sudo apt-get -y install libopenmpi-dev || true
      sudo apt-get install -y ninja-build

      # OpenGL packages
      sudo apt-get install -y xlibmesa-glu-dev libglew1.5-dev libftgl-dev

      # Install dependencies to install Python with PyEnv
      sudo apt-get install -y libssl-dev zlib1g-dev libbz2-dev \
        libreadline-dev libsqlite3-dev wget curl llvm \
        xz-utils tk-dev libffi-dev liblzma-dev python-openssl git

      sudo apt install -y libxt-dev freeglut3-dev

      if [ "$PV_FLAVOR" = "nvidia-headless" ]; then
        sudo apt install -y libegl1-mesa-dev libegl1 libgl1 libglvnd libglvnd-dev
      fi
    fi

    # update cmake
    URL="https://cmake.org/files/v3.19/cmake-3.19.3-Linux-x86_64.tar.gz"
    DownloadTarAndExtract $URL $WORKING_DIR/cmake-3.19.3 1
    export PATH=$WORKING_DIR/cmake-3.19.3/bin:$PATH

    # pyenv
    if [ -z "$SKIP_PYENV" ]; then
      # Install pyenv and python 3.8.0
      curl https://pyenv.run | bash
      export PATH="$HOME/.pyenv/bin:$PATH"
      eval "$(pyenv init -)"
      env PYTHON_CONFIGURE_OPTS="--enable-shared" pyenv install 3.8.0
    else
      eval "$(pyenv init -)"
    fi
    pyenv shell 3.8.0

    BDM_OS_VERS=${BDM_OS}
  else
    brew update --preinstall
    brew install bash git cmake ninja swig python@3.9 libomp open-mpi git-lfs

    MACOS_VERS=`sw_vers | sed -n 's/ProductVersion://p' | cut -d . -f 1-2 | sed -e 's/^[[:space:]]*//'`
    MACOS_ARCH=`arch`
    BDM_OS_VERS=${BDM_OS}-${MACOS_VERS}-${MACOS_ARCH}
  fi
fi

# qt
if [ -z "$SKIP_QT" ]; then
  QT_TAR=qt_${QT_VERSION}_${BDM_OS_VERS}.tar.gz
  mkdir -p $QT_INSTALL_DIR
  QT_TAR_FILE="${QT_INSTALL_DIR}/${QT_TAR}"

  if [ -n "$BDM_LOCAL_LFS" ]; then
    tar -zxf "${BDM_LOCAL_LFS}third-party/${QT_TAR}" -C "$QT_INSTALL_DIR"
    cd ${QT_INSTALL_DIR}
  else
    QT_URL=http://cern.ch/biodynamo-lfs/third-party/${QT_TAR}
    wget --progress=dot:giga -O $QT_TAR_FILE $QT_URL
    cd ${QT_INSTALL_DIR}
    tar -zxf $QT_TAR
  fi
fi
