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
  This script installs the the prerequisites of BioDynaMo, but not BioDynaMo
  itself. Script install.sh installs both prerequisites and BioDynaMo.
Arguments:
  \$1 path to the biodynamo project directory"
  exit 1
fi

set -e

# set parameter
BDM_PROJECT_DIR=$1
BDM_OS=ubuntu-16.04
BDM_INSTALL_DIR=/opt/biodynamo

# include util functions
. $BDM_PROJECT_DIR/cmake/installation/common/util.sh

function InstallCmake {
  local URL="https://cmake.org/files/v3.6/cmake-3.6.3-Linux-x86_64.tar.gz"
  DownloadTarAndExtract $URL $THIRD_PARTY_DIR/cmake-3.6.3 1
  # cmake/bin is added to PATH in the biodynamo environment script
}

function Install {
  echo "Start installation of prerequisites..."

  THIRD_PARTY_DIR=$BDM_INSTALL_DIR/third_party

  # Remove everything in ${THIRD_PARTY_DIR} if it exists already.
  # Might contain outdated dependencies
  if [ -d "${THIRD_PARTY_DIR}" ]; then
    sudo rm -rf "${THIRD_PARTY_DIR}/*"
  else
    sudo mkdir -p $THIRD_PARTY_DIR
  fi

  # install `apt-add-repository and wget if not already installed
  # (missing on docker image)
  sudo apt-get install -y software-properties-common wget

  # add repository for clang-3.9
  wget -O - http://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
  sudo apt-add-repository -y "deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-3.9 main"
  sudo apt-get update

  # install packages
  sudo apt-get -y install freeglut3-dev
  sudo apt-get -y install git valgrind python python3 python2.7-dev lcov
  sudo apt-get -y install gcc-5 g++-5 make cmake
  sudo apt-get -y install clang-3.9 clang-format-3.9 clang-tidy-3.9 libomp-dev
  sudo apt-get -y install doxygen graphviz
  sudo apt-get -y install python-pip
  pip install --user mkdocs
  pip install --user mkdocs-material

  # copy environment script
  BDM_ENVIRONMENT_FILE=$BDM_INSTALL_DIR/biodynamo_env.sh
  sudo cp $BDM_PROJECT_DIR/cmake/installation/common/biodynamo_linux_env.sh $BDM_ENVIRONMENT_FILE

  # install CMake higher than the required version
  CMAKE_VERSION_R=3.3
  CMAKE_VERSION_I=`cmake --version | grep -m1 "" | sed -e 's/\<cmake version\>//g' -e "s/ //g"`
  if hash cmake 2>/dev/null; then
    rv=( ${CMAKE_VERSION_R//./ } )
    iv=( ${CMAKE_VERSION_I//./ } )
    if ! ((${iv[0]} >= ${rv[0]} && ${iv[1]} >= ${rv[0]})); then
      InstallCmake
    fi
  else
    InstallCmake
  fi

  DownloadTarFromCBAndExtract $BDM_OS root.tar.gz $THIRD_PARTY_DIR
  DownloadTarFromCBAndExtract $BDM_OS paraview.tar.gz $THIRD_PARTY_DIR/paraview
  DownloadTarFromCBAndExtract $BDM_OS qt.tar.gz $THIRD_PARTY_DIR/qt

  # temporal workaround to avoid libprotobuf error for paraview
  # use only until patched archive has been uploaded
  sudo rm $THIRD_PARTY_DIR/qt/plugins/platformthemes/libqgtk3.so
  sudo rm $THIRD_PARTY_DIR/qt/lib/cmake/Qt5Gui/Qt5Gui_QGtk3ThemePlugin.cmake
  sudo touch $THIRD_PARTY_DIR/qt/lib/cmake/Qt5Gui/Qt5Gui_QGtk3ThemePlugin.cmake

  UpdateSourceBdmVariable $BDM_ENVIRONMENT_FILE

  echo "Installation of prerequisites finished successfully!"
  EchoFinishThisStep
  echo ""
}

RequireSudo

# ask user if she really wants to perform this changes
PromptUser "This script adds the following package repository:
'deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-3.9 main'
and installs the required packages. Open this file with an editor to see which
packages will be installed.
Do you want to continue?" Install
