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

# This file contains functions required in various install scripts.
# (Thus reducing code duplication)

SCRIPTPATH="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

#include echo.sh
. $SCRIPTPATH/echo.sh

# This function asks the user for the sudo password and keeps it in the cache
# for the duration of the script execution.
function RequireSudo {
  # Workaround for: https://github.com/travis-ci/travis-ci/issues/9608
  if [ "$TRAVIS_OS_NAME" != "osx" ]; then
    # Print message if sudo password needs to be entered
    sudo -n true &>/dev/null || echo "Some commands in this script require sudo priviledges"
    sudo -v
    # keep sudo priviledges alive
    # kill -0 "$$": checks if parent process is still running
    # https://github.com/mathiasbynens/dotfiles/blob/master/.macos
    while true; do sudo -n true; sleep 10; kill -0 "$$" || exit; done 2>/dev/null &
  fi
}

# Detects the linux flavour using `lsb_release`.
# Returns a lower case version of `distributor-release`.
function DetectLinuxFlavour {
  DISTRIBUTOR=$(lsb_release -si)
  RELEASE=$(lsb_release -sr)
  OS="${DISTRIBUTOR}-${RELEASE}"
  echo $OS | awk '{print tolower($0)}'
}

# Function that detects the OS
# Returns linux flavour or osx.
# This function prints an error and exits if is not linux or macos.
function DetectOs {
  # detect operating system
  if [ $TRAVIS ]; then
    echo "travis-${TRAVIS_OS_NAME}"
  elif [ `uname` = "Linux" ]; then
    # linux
    DetectLinuxFlavour
  elif [ `uname` = "Darwin" ]; then
    # macOS
    echo osx
  else
    echo "ERROR: Operating system `uname` is not supported."
    exit 1
  fi
}

# This function checks if the given operating system is supported. If not, it
# prints an error message, a list of supported systems; and exits the script.
# Arguments:
#   $1 path to biodynamo installation src folder (util/installation)
#   $2 OS identifier e.g. ubuntu-16.04 (see DetectOs)
function CheckOsSupported {
  if [[ $# -ne 2 ]]; then
    echo "ERROR in CheckOsSupported: Wrong number of arguments"
    exit 1
  fi

  local BDM_INSTALL_SRC=$1
  local BDM_OS=$2

  local BDM_INSTALL_OS_SRC=$BDM_INSTALL_SRC/$BDM_OS

  if [ ! -d "$BDM_INSTALL_OS_SRC" ]; then
    echo "ERROR: This operating system (${BDM_OS}) is not supported"
    echo "Supported operating systems are: "
    pushd $BDM_INSTALL_SRC > /dev/null
    ls -d */ | grep -v common | sed 's|/||g'
    popd > /dev/null
    exit 1;
  fi
}

# Download a tar file extracts the contents into the destination folder.
# Arguments:
#   $1 url
#   $2 destination directory in which the archive will be extracted
#   $3 strip components [optional] (default 0)
function DownloadTarAndExtract {
  if [[ $# -lt 2 ]]; then
    echo "ERROR in DownloadTarAndExtract: Wrong number of arguments"
    exit 1
  fi

  local URL=$1
  local DEST=$2
  local STRIP_COMP=${3:-0}

  local TMP_DEST=/tmp/$RANDOM

  rm $TMP_DEST 2>/dev/null || true
  mkdir -p $DEST
  if [ $BDM_LOCAL_LFS ]  && [[ "$URL" = /* ]]; then
    tar -xzf $URL --strip=$STRIP_COMP -C $DEST
  else
    wget --progress=dot:giga -O $TMP_DEST $URL
    tar -xzf $TMP_DEST --strip=$STRIP_COMP -C $DEST
    rm $TMP_DEST
  fi
}

# Returns a biodynamo lfs download link.
# If the environment variable BDM_LOCAL_LFS is set to a local copy of LFS,
# this function returns a local path to the requested file.
# Arguments:
#   $1 directory
#   $2 file
function BdmLfsLink {
  if [[ $# -ne 2 ]]; then
    echo "ERROR in BdmLfsLink: Wrong number of arguments"
    exit 1
  fi
  if [ ! $BDM_LOCAL_LFS ]; then
    echo "http://cern.ch/biodynamo-lfs/third-party/${1}/${2}"
  else
    echo "${BDM_LOCAL_LFS}/${1}/${2}"
  fi
}

# Download a tar file from CERNBox and extracts the contents into the
# destination folder.
# Arguments:
#   $1 CERNBox directory e.g. ubuntu-16.04
#   $2 File e.g. root.tar.gz
#   $3 destination directory in which the archive will be extracted
function DownloadTarFromCBAndExtract {
  if [[ $# -ne 3 ]]; then
    echo "ERROR in DownloadTarFromCBAndExtract: Wrong number of arguments"
    exit 1
  fi

  local DIR=$1
  local FILE=$2
  local DEST=$3

  URL=$(BdmLfsLink $DIR $FILE)

  DownloadTarAndExtract $URL $DEST
}

# Returns the number of CPU cores including hyperthreads
function CPUCount {
  if [ `uname` = "Darwin" ]; then
    sysctl -n hw.ncpu
  else
    grep -c ^processor /proc/cpuinfo
  fi
}

# This function performs a clean build in the given build directory.
# Creates the build dir if it does not exist.
# CMake flags can be added by setting the environment variable BDM_CMAKE_FLAGS.
# Arguments:
#   $1 build directory; must be one level below CMakeLists.txt
#      (-> $1/../CMakeLists.txt exists)
function CleanBuild {
  if [[ $# -ne 1 ]]; then
    echo "ERROR in CleanBuild: Wrong number of arguments"
    exit 1
  fi

  local BUILD_DIR=$1

  if [ -d "${BUILD_DIR}" ]; then
    rm -rf $BUILD_DIR/*
  else
    mkdir -p $BUILD_DIR
  fi
  cd $BUILD_DIR
  echo "CMAKEFLAGS $BDM_CMAKE_FLAGS"
  cmake $BDM_CMAKE_FLAGS .. && make -j$(CPUCount) && make install
}

# Return absolute path.
# If absolute path is given as paramter it is returned as is. Otherwise it gets converted.
# Arguments:
#  $1 path (absolute or relative)
function GetAbsolutePath {
  if [[ $# -ne 1 ]]; then
    echo "ERROR in GetAbsolutePath: Wrong number of arguments"
    exit 1
  fi

  if [[ "$1" = /* ]]; then
    echo "$1"
  else
    echo "${PWD}/${1}"
  fi
}

# This function calls the OS specific version of the given script.
# Arguments:
#   $1 biodynamo project directory
#   $2 filename of the script that should be executed
#   $@ arguments that should be passed to the script
function CallOSSpecificScript {
  if [[ $# -lt 2 ]]; then
    echo "ERROR in CallOSSpecificScript: Wrong number of arguments"
    exit 1
  fi

  local BDM_PROJECT_DIR=$1
  shift
  local BDM_SCRIPT_FILE=$1
  shift

  local BDM_INSTALL_SRC=$BDM_PROJECT_DIR/util/installation

  # detect os
  local BDM_OS=$(DetectOs)
  local BDM_INSTALL_OS_SRC=$BDM_INSTALL_SRC/$BDM_OS

  # check if this system is supported
  CheckOsSupported $BDM_INSTALL_SRC $BDM_OS

  # check if script exists for the detected OS
  local BDM_SCRIPTPATH=$BDM_INSTALL_OS_SRC/$BDM_SCRIPT_FILE
  if [ ! -f $BDM_SCRIPTPATH ]; then
    echo "ERROR in CallOSSpecificScript: $BDM_SCRIPTPATH does not exist"
    exit 1
  fi

  $BDM_SCRIPTPATH $@
}

# Return the bashrc file based on the current operating system
function BashrcFile {
  if [ `uname` = "Darwin" ]; then
    echo ~/.bash_profile
  elif [ `uname` = "Linux" ]; then
    echo ~/.bashrc
  else
    echo "ERROR: Operating system `uname` is not supported."
    exit 1
  fi
}

# Print message that tells the user to reload the bash and source the environment.
# Arguments:
#   $1 biodynamo install directory
function EchoFinishThisStep {
  if [[ $# -ne 1 ]]; then
    echo "ERROR in EchoFinishThisStep: Wrong number of arguments"
    exit 1
  fi

  EchoInfo "To complete this step execute:"
  EchoInfo "    ${BDM_ECHO_BOLD}${BDM_ECHO_UNDERLINE}source $1/biodynamo-env.sh"
  EchoInfo "This command must be executed in every terminal before you build or use BioDynaMo."
  EchoInfo "To avoid this additional step add it to your $(BashrcFile) file:"
  EchoInfo "    echo \"source $1/biodynamo-env.sh\" >> $(BashrcFile)"
}

# This function prompts the user for the biodynamo installation direcotory
# providing an option to accept a default.
function SelectInstallDir {
  if [[ $# -ne 0 ]]; then
    echo "ERROR in SelectInstallDir: Wrong number of arguments"
    exit 1
  fi

  local DEFAULT=~/.bdm
  local MSG="Do you want to use the default installation directory ($DEFAULT) ?"
  while true; do
    read -p "$MSG (y/n) " yn
    case $yn in
      [Yy]* )
        echo $DEFAULT
        break
      ;;
      [Nn]* )
        read -p "Please enter the biodynamo installation directory: " INSTALL_DIR
        # check if the user entered a path relative to home ~/
        # this is not automaticall expanded
        case "$INSTALL_DIR" in "~/"*)
          INSTALL_DIR="${HOME}/${INSTALL_DIR#"~/"}"
        esac
        echo $INSTALL_DIR
        break
      ;;
      * ) echo "Please answer yes or no.";;
    esac
  done
}

# This function checks if the given directory is valid, if it needs to be
# created, or if containing files need to be removed.
# Arguments:
#   $1 biodynamo install directory
function PrepareInstallDir {
  if [[ $# -ne 1 ]]; then
    echo "ERROR in PrepareInstallDir: Wrong number of arguments"
    exit 1
  fi

  local BDM_INSTALL_DIR=$1

  if [[ $BDM_INSTALL_DIR == "" ]] || [[ $BDM_INSTALL_DIR == / ]] || [[ $BDM_INSTALL_DIR == /usr* ]]; then
    echo "ERROR: Invalid installation directory: $BDM_INSTALL_DIR"
    exit 1
  elif [ ! -d $BDM_INSTALL_DIR ]; then
    # Install directory does not exist
    echo "installdir " $BDM_INSTALL_DIR
    mkdir -p $BDM_INSTALL_DIR
  elif [ ! -z "$(ls -A $BDM_INSTALL_DIR)" ]; then
    # Install directory is not empty
    MSG="The chosen installation directory is not empty!
The installation process will remove all files in $BDM_INSTALL_DIR! Do you want to continue?"
    while true; do
      read -p "$MSG (y/n) " yn
      case $yn in
        [Yy]* )
          rm -rf $BDM_INSTALL_DIR/*
          break
        ;;
        [Nn]* )
        exit 1;;
        * ) echo "Please answer yes or no.";;
      esac
    done
  fi
}

# This function copies the environment script to the install path and
# corrects the BDM_INSTALL_DIR environment variable inside the script.
# Arguments:
#   $1 environment source script
#   $2 biodynamo install directory
function CopyEnvironmentScript {
  if [[ $# -ne 2 ]]; then
    echo "ERROR in CopyEnvironmentScript: Wrong number of arguments"
    exit 1
  fi

  local ENV_SRC=$1
  local BDM_INSTALL_DIR=$2

  local ENV_DEST=$BDM_INSTALL_DIR/biodynamo-env.sh

  local TMP_ENV=${ENV_DEST}_$RANDOM

  cat $ENV_SRC | sed "s|export BDM_INSTALL_DIR=@CMAKE_INSTALL_PREFIX@|export BDM_INSTALL_DIR=${BDM_INSTALL_DIR}|g" >$TMP_ENV
  mv $TMP_ENV $ENV_DEST
}
