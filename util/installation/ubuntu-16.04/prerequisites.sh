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

# Don't hardcode ubuntu-16.04 here, so we can reuse this script from
# e.g. ubuntu-18.04
BDM_OS=$(DetectOs)

function InstallCmake {
  local URL="https://cmake.org/files/v3.6/cmake-3.6.3-Linux-x86_64.tar.gz"
  DownloadTarAndExtract $URL $1/cmake-3.6.3 1
  # cmake/bin is added to PATH in the biodynamo environment script
}

function InstallPackages {
  INSTALL_PACKAGES="freeglut3-dev  git valgrind python python3 python2.7-dev lcov \
  gcc g++ make cmake clang-3.9 clang-format-3.9 clang-tidy-3.9 libomp-dev \
  doxygen graphviz python-pip"

  ADD_REPOSITORY='deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-3.9 main'

  EchoInfo "This script uses apt to install:"
  for p in $INSTALL_PACKAGES; do
    EchoInfo "  $p"
  done | column
  EchoInfo ""
  EchoInfo "It adds the repository:"
  EchoInfo "  $ADD_REPOSITORY"
  EchoInfo ""
  EchoInfo "It uses pip to install mkdocs and mkdocs-material."
  EchoInfo ""
  EchoInfo "Open \"util/installation/ubuntu-16.04/prerequisites.sh\" for more information."
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
    wget -O - http://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
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

  DownloadTarFromCBAndExtract $BDM_OS root.tar.gz $THIRD_PARTY_DIR/root
  DownloadTarFromCBAndExtract $BDM_OS paraview.tar.gz $THIRD_PARTY_DIR/paraview
  DownloadTarFromCBAndExtract $BDM_OS qt.tar.gz $THIRD_PARTY_DIR/qt
  DownloadTarFromCBAndExtract $BDM_OS ray.tar.gz $THIRD_PARTY_DIR
  pip install --user "${THIRD_PARTY_DIR}/ray/ray-0.5.0-cp27-cp27mu-linux_x86_64.whl"
  rm "${THIRD_PARTY_DIR}/ray/ray-0.5.0-cp27-cp27mu-linux_x86_64.whl"

  # temporal workaround to avoid libprotobuf error for paraview
  # use only until patched archive has been uploaded
  rm $THIRD_PARTY_DIR/qt/plugins/platformthemes/libqgtk3.so
  rm $THIRD_PARTY_DIR/qt/lib/cmake/Qt5Gui/Qt5Gui_QGtk3ThemePlugin.cmake
  touch $THIRD_PARTY_DIR/qt/lib/cmake/Qt5Gui/Qt5Gui_QGtk3ThemePlugin.cmake

  EchoSuccess "Installation of prerequisites finished successfully!"
  EchoFinishThisStep $BDM_INSTALL_DIR
  echo ""
}

Install
