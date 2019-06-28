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

# include util functions
. $BDM_PROJECT_DIR/util/installation/common/util.sh

# Don't hardcode OS here, so we can reuse this script from
# e.g. centos-...
BDM_OS=$(DetectOs)

function InstallCmake {
  local URL="https://cmake.org/files/v3.6/cmake-3.6.3-Linux-x86_64.tar.gz"
  DownloadTarAndExtract $URL $1/cmake-3.6.3 1
  export PATH=$1/cmake-3.6.3/bin:${PATH}
}

function InstallPackages {
  # libXt-devel libXext-devel are required to build the paraview plugin
  # on ubuntu those packages are installed together with freeglut3-dev
  INSTALL_PACKAGES="git valgrind  \
    freeglut-devel libXt-devel libXext-devel \
    python python-pip rh-python36 python-devel  \
    devtoolset-7-gcc* make cmake openmpi3-devel gcc-c++"

  EchoInfo "This script uses yum to install centos-release-scl, epel-release, and:"
  for p in $INSTALL_PACKAGES; do
    EchoInfo "  $p"
  done | column
  EchoInfo ""
  EchoInfo "It uses pip to install mkdocs and mkdocs-material."
  EchoInfo ""
  EchoInfo "Open \"util/installation/centos-7.6.1810/prerequisites.sh\" for more information."
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
    # add repository for clang-3.9
    sudo yum update -y

    # install packages
    sudo yum -y install centos-release-scl epel-release
    sudo yum -y install $INSTALL_PACKAGES

    # activate mpi
    . /etc/profile.d/modules.sh
    module load mpi

  fi

}

function Install {
  echo "Start installation of prerequisites..."

  export BDM_INSTALL_DIR=$(SelectInstallDir)
  PrepareInstallDir $BDM_INSTALL_DIR
  THIRD_PARTY_DIR=$BDM_INSTALL_DIR/third_party

  InstallPackages

  # install CMake higher than the required version
  CMAKE_VERSION_R=3.3
  CMAKE_VERSION_I=`cmake --version | grep -m1 "" | sed -e 's/\<cmake version\>//g' -e "s/ //g"`
  if hash cmake 2>/dev/null; then
    rv=( ${CMAKE_VERSION_R//./ } )
    iv=( ${CMAKE_VERSION_I//./ } )
    if ! ((${iv[0]} >= ${rv[0]} && ${iv[1]} >= ${rv[0]})); then
      InstallCmake $THIRD_PARTY_DIR
    fi
  else
    InstallCmake $THIRD_PARTY_DIR
  fi

  EchoSuccess "Installation of prerequisites finished successfully!"
  EchoFinishThisStep $BDM_INSTALL_DIR
  echo ""
}

Install
