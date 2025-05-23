#!/bin/bash
# -----------------------------------------------------------------------------
#
# Copyright (C) 2021 CERN & University of Surrey for the benefit of the
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

if [[ $# -ne 1 ]]; then
  echo "ERROR: Wrong number of arguments.
Description:
    This script installs the prerequisites of BioDynaMo, but not BioDynaMo
    itself. Script install.sh installs both prerequisites and BioDynaMo.
Arguments:
    <install_type>  all/required. If all is specified, then this script
                    will install all the prerequisites."
  exit 1
fi

BDM_PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../../.."

if [ `arch` == "i386" -a ! -x "/usr/local/bin/brew" ]; then
   echo "First install the Homebrew macOS package manager from https://brew.sh"
   exit 1
elif [ `arch` == "arm64" -a ! -x "/opt/homebrew/bin/brew" ]; then
   echo "First install the Homebrew macOS package manager from https://brew.sh"
   exit 1
fi

if [ ! -x "/usr/bin/git" ]; then
   echo "First install Xcode (from the App Store) and the command line tools"
   echo "using the command \"xcode-select --install\"."
   exit 1
fi

# Test if Xcode is installed
# https://www.kindacode.com/article/check-if-xcode-is-installed-on-mac-via-command-line/
if [ ! "xcode-select -p" ]; then
  echo "Please make sure that Xcode and the command line tools are installed."
  echo "Our checks indicate that they are not yet on your system."
  echo "For more information regarding BioDynamo's prerequisites, please check"
  echo "https://biodynamo.org/docs/userguide/prerequisites/#macos"
  exit 1
fi 

# Install and upgrade required packages
brew install \
  $(cat $BDM_PROJECT_DIR/util/installation/osx/package_list_required) || true

wget https://cmake.org/files/v3.24/cmake-3.24.1-macos-universal.tar.gz
tar -xzf cmake-3.24.1-macos-universal.tar.gz
sudo mv cmake-3.24.1-macos-universal/CMake.app /Applications/
sudo ln -sf /Applications/CMake.app/Contents/bin/cmake /usr/local/bin/cmake

# Install the optional packages
if [ $1 == "all" ]; then
    # Fix jinja2 version because of failing build target `notebooks` on 
    # macOS System CI.
    PIP_PACKAGES="markupsafe==2.0.1 nbformat jupyter metakernel jupyterlab jinja2==3.0"
    # Don't install --user: the packages should end up in the PYENV_ROOT directory
    python3.9 -m pip install $PIP_PACKAGES
    brew install \
      $(cat $BDM_PROJECT_DIR/util/installation/osx/package_list_extra) || true
fi

# Test installation of brew formulae such that the user gets feedback if 
# his/her brew installation did not work as expected.
brew_required=0
for package in $(cat $BDM_PROJECT_DIR/util/installation/osx/package_list_required)
do
  brew list $package &> /dev/null
  if [ $? -eq 1 ]; then
    brew_required=1
    echo "Warning: required brew formula $package was not installed."
  fi
done
echo ""
if [ $brew_required -eq 0 ]; then
  echo "All required brew fomulae were installed successfully."
else
  echo "Some required brew formulae could not be installed. Please check your"
  echo "log and make sure your homebrew works properly."
fi

# Test installation of optional brew formulae.
if [ $1 == "all" ]; then
  # Test if all extra  packages were really successfully installed.
  brew_extra=0
  for package in $(cat $BDM_PROJECT_DIR/util/installation/osx/package_list_extra)
  do
    brew list $package &> /dev/null
    if [ $? -eq 1 ]; then
      brew_extra=1
      echo "Warning: optional brew formula $package was not installed."
    fi
  done
  if [ $brew_extra -eq 0 ]; then
    echo "All optional brew fomulae were installed successfully."
  else
    echo "Some optional brew formulae could not be installed. Please check your"
    echo "log and make sure your homebrew works properly."
  fi
fi

# Recommend user to upgrade to the latest package versions
echo ""
echo "Maybe you have seen errors of the type 'Error: <package> is already" 
echo "installed'. If such an error occured in the context of one of the packages"
echo "listed on top, consider upgrading with 'brew upgrade <package>'."

exit 0
