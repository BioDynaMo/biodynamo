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
BDM_OS=travis-osx
BDM_INSTALL_DIR=/opt/biodynamo

# include util functions
. $BDM_PROJECT_DIR/util/installation/common/util.sh

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
  brew update >& /dev/null
  brew install doxygen
  brew install valgrind
  brew install cloc
  brew install qt
  brew upgrade python3 || true
  brew install python@2 || true

  brew install llvm  # get clang 5.0
  brew upgrade cmake || true
  #  for mkdocs
  export PATH=$PATH:~/Library/Python/2.7/bin
  pip2 install --user mkdocs
  pip2 install --user mkdocs-material

  # copy environment script
  BDM_ENVIRONMENT_FILE=$BDM_INSTALL_DIR/biodynamo_env.sh
  sudo cp $BDM_PROJECT_DIR/util/installation/common/biodynamo_macos_env.sh $BDM_ENVIRONMENT_FILE

  # install third_party dependencies
  DownloadTarFromCBAndExtract $BDM_OS root.tar.gz $THIRD_PARTY_DIR
  DownloadTarFromCBAndExtract $BDM_OS paraview.tar.gz $THIRD_PARTY_DIR/paraview

  # misc
  # copy the omp.h file to our CMAKE_PREFIX_PATH
  sudo mkdir -p /usr/local/Cellar/biodynamo
  OMP_V=`/usr/local/opt/llvm/bin/llvm-config --version`
  sudo cp -f /usr/local/opt/llvm/lib/clang/$OMP_V/include/omp.h /usr/local/Cellar/biodynamo

  UpdateSourceBdmVariable $BDM_ENVIRONMENT_FILE

  echo "Installation of prerequisites finished successfully!"
  EchoFinishThisStep
  echo ""
}

RequireSudo

# ask user if she really wants to perform this changes
PromptUser "This script installs the required packages for BioDynaMo.
Open this file with an editor to see which packages will be installed.
Do you want to continue?" Install
