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
BDM_OS=travis-osx

# include util functions
. $BDM_PROJECT_DIR/util/installation/common/util.sh

function InstallPackages {
  BREW_INSTALL_PACKAGES="doxygen cloc llvm python@2"
  BREW_UPGRADE_PACKAGES="cmake python"

  EchoInfo "This script uses brew to install:"
  for p in $BREW_INSTALL_PACKAGES; do
    EchoInfo "  $p"
  done | column
  EchoInfo ""
  EchoInfo "It will upgrade:"
  for p in $BREW_UPGRADE_PACKAGES; do
    EchoInfo "  $p"
  done | column
  EchoInfo ""

  EchoInfo "It uses pip2 to install mkdocs and mkdocs-material."
  EchoInfo ""
  EchoInfo "Open \"util/installation/travis-osx/prerequisites.sh\" for more information."
  EchoInfo ""
  EchoInfo "These commands do NOT require sudo rights. If you are sure that these packages have already been installed, you can skip this step."
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
    brew update >& /dev/null
    brew install python || true # if python is not already installed we need to install it before upgrading it
    brew upgrade $BREW_UPGRADE_PACKAGES || true
    brew install $BREW_INSTALL_PACKAGES || true
    #  for mkdocs
    export PATH=$PATH:~/Library/Python/2.7/bin
    pip2 install --user mkdocs
    pip2 install --user mkdocs-material
  fi
}

function Install {
  echo "Start installation of prerequisites..."

  # determine install dir
  export BDM_INSTALL_DIR=$(SelectInstallDir)
  PrepareInstallDir $BDM_INSTALL_DIR
  THIRD_PARTY_DIR=$BDM_INSTALL_DIR/third_party

  InstallPackages

  # copy environment script
  CopyEnvironmentScript $BDM_PROJECT_DIR/util/installation/common/biodynamo-macos-env.sh $BDM_INSTALL_DIR

  # install third_party dependencies
  DownloadTarFromCBAndExtract $BDM_OS qt.tar.gz $THIRD_PARTY_DIR/qt
  DownloadTarFromCBAndExtract $BDM_OS root.tar.gz $THIRD_PARTY_DIR/root
  DownloadTarFromCBAndExtract $BDM_OS paraview.tar.gz $THIRD_PARTY_DIR/paraview

  # misc
  # copy the omp.h file to our CMAKE_PREFIX_PATH
  OMP_V=`/usr/local/opt/llvm/bin/llvm-config --version`
  mkdir -p $BDM_INSTALL_DIR/biodynamo/include
  cp -f /usr/local/opt/llvm/lib/clang/$OMP_V/include/omp.h $BDM_INSTALL_DIR/biodynamo/include

  EchoSuccess "Installation of prerequisites finished successfully!"
  EchoFinishThisStep $BDM_INSTALL_DIR
  echo ""
}

Install
