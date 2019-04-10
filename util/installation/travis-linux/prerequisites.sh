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

#!/bin/bash

if [[ $# -ne 0 ]]; then
  echo "ERROR: Wrong number of arguments.
Description:
    This script installs the prerequisites of BioDynaMo, but not BioDynaMo
    itself. Script install.sh installs both prerequisites and BioDynaMo.
No Arguments"
  exit 1
fi

set -e

# set parameter
BDM_PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../../.."
BDM_OS=travis-linux

# include util functions
. $BDM_PROJECT_DIR/util/installation/common/util.sh

function InstallCmake {
  local URL="https://cmake.org/files/v3.6/cmake-3.6.3-Linux-x86_64.tar.gz"
  DownloadTarAndExtract $URL $1/cmake-3.6.3 1
  # cmake/bin is added to PATH in the biodynamo environment script
}

function InstallPackages {
  INSTALL_PACKAGES="freeglut3-dev gcc-5 g++-5 valgrind doxygen graphviz cloc  \
  libiomp-dev clang-3.9 clang-format-3.9 clang-tidy-3.9 python2.7 libnuma-dev \
  libtbb-dev mpich libmpich-dev"

  ADD_REPOSITORY='deb http://apt.llvm.org/trusty/ llvm-toolchain-trusty-3.9 main'

  EchoInfo "This script uses apt to install:"
  for p in $INSTALL_PACKAGES; do
    EchoInfo "  $p"
  done | column
  EchoInfo ""
  EchoInfo "It adds the repository:"
  EchoInfo "  ppa:ubuntu-toolchain-r/test"
  EchoInfo "  ppa:jonathonf/python-2.7"
  EchoInfo "  $ADD_REPOSITORY"
  EchoInfo ""
  EchoInfo "It uses pip to install mkdocs and mkdocs-material."
  EchoInfo ""
  EchoInfo "Open \"util/installation/travis-linux/prerequisites.sh\" for more information."
  EchoInfo ""
  EchoInfo "These commands require sudo rights. If you are sure that these packages have already been installed, you can skip this step."
  EchoInfo ""
  EchoInfo "Do you want to perform these changes? (yes/no/skip)?"

  # ask user if she really wants to perform this changes
  unset INSTALL
  while true; do
    read -p "" yn
    case $yn in
      [Yy]* ) echo "Installing packages..." ; INSTALL=true; break;;
      [Ss]* ) echo "Skipping package installation"; break;;
      [Nn]* ) echo "Aborting"; exit 1;;
          * ) echo "Please answer yes, no or skip.";;
    esac
  done

  if [ $INSTALL ]; then
    sudo -v
    sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test  # gcc-5
    # add repository for clang-3.9
    wget -O - http://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
    sudo add-apt-repository ppa:jonathonf/python-2.7 # python 2.7.14
    sudo apt-add-repository -y "$ADD_REPOSITORY"
    sudo apt-get update

    # install packages
    sudo apt-get -y install $INSTALL_PACKAGES
    pip install --user mkdocs
    pip install --user mkdocs-material
  fi

}

function Install {
  echo "Start installation of prerequisites..."

  export BDM_INSTALL_DIR=$(SelectInstallDir)
  PrepareInstallDir $BDM_INSTALL_DIR
  THIRD_PARTY_DIR=$BDM_INSTALL_DIR/third_party

  InstallPackages

  # copy environment script
  CopyEnvironmentScript $BDM_PROJECT_DIR/util/installation/common/biodynamo-linux-env.sh $BDM_INSTALL_DIR

  InstallCmake $THIRD_PARTY_DIR

  # install third_party dependencies
  DownloadTarFromCBAndExtract $BDM_OS root.tar.gz $THIRD_PARTY_DIR/root
  DownloadTarFromCBAndExtract $BDM_OS paraview-v5.6.0.tar.gz $THIRD_PARTY_DIR/paraview
  DownloadTarFromCBAndExtract $BDM_OS qt.tar.gz $THIRD_PARTY_DIR/qt

  EchoSuccess "Installation of prerequisites finished successfully!"
  EchoFinishThisStep $BDM_INSTALL_DIR
  echo ""
}

Install
