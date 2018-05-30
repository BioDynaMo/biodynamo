#!/bin/bash

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
BDM_OS=travis-linux
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

  # Install packages
  sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test  # gcc-5
  wget -O - http://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
  sudo apt-add-repository -y "deb http://apt.llvm.org/trusty/ llvm-toolchain-trusty-3.9 main"
  sudo apt-get update
  sudo apt-get -y install openmpi-bin libopenmpi-dev
  sudo apt-get -y install freeglut3-dev
  sudo apt-get -y install gcc-5 g++-5
  sudo apt-get -y install valgrind
  sudo apt-get -y install doxygen
  sudo apt-get -y install cloc
  sudo apt-get -y install libiomp-dev
  sudo apt-get -y install clang-3.9 clang-format-3.9 clang-tidy-3.9

  InstallCmake

  # install third_party dependencies
  DownloadTarFromCBAndExtract $BDM_OS root.tar.gz $THIRD_PARTY_DIR
  DownloadTarFromCBAndExtract $BDM_OS paraview.tar.gz $THIRD_PARTY_DIR/paraview
  DownloadTarFromCBAndExtract $BDM_OS qt.tar.gz $THIRD_PARTY_DIR/qt

  # needed for Catalyst
  sudo ln -s /usr/lib/libmpi.so /usr/local/lib/libmpi.so
  sudo ln -s /usr/lib/libmpi.so /usr/local/lib/libmpi.so.12

  # copy environment script
  BDM_ENVIRONMENT_FILE=$BDM_INSTALL_DIR/biodynamo_env.sh
  sudo cp $BDM_PROJECT_DIR/cmake/installation/common/biodynamo_linux_env.sh $BDM_ENVIRONMENT_FILE

  UpdateSourceBdmVariable $BDM_ENVIRONMENT_FILE

  echo "Installation of prerequisites finished successfully!"
  EchoFinishThisStep
  echo ""
}

RequireSudo

# ask user if she really wants to perform this changes
PromptUser "This script adds the following package repository:
'deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-3.9 main',
'ppa:ubuntu-toolchain-r/test',
and installs the required packages. Open this file with an editor to see which
packages will be installed.
Do you want to continue?" Install
