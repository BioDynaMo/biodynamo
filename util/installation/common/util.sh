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
  # Print message if sudo password needs to be entered
  sudo -n true &>/dev/null || echo "Some commands in this script require sudo priviledges"
  sudo -v
  # keep sudo priviledges alive
  # kill -0 "$$": checks if parent process is still running
  # https://github.com/mathiasbynens/dotfiles/blob/master/.macos
  while true; do sudo -n true; sleep 10; kill -0 "$$" || exit; done 2>/dev/null &
}

# Wait for user input
WaitForUser() {
  read -n1 -rsp $'Press any key to continue or Ctrl+C to exit...\n' </dev/tty
}

# Function that detects the OS
# Returns linux flavour or osx.
# This function prints an error and exits if is not linux or macos.
function DetectOs {
  # detect operating system
  if [ `uname` = "Linux" ]; then
    # linux
    DISTRIBUTOR=$(grep -oP '(?<=^ID=).+' /etc/os-release | tr -d '"')
    RELEASE=$(grep -oP '(?<=^VERSION_ID=).+' /etc/os-release | tr -d '"')
    OS="${DISTRIBUTOR}-${RELEASE}"
    echo $OS | awk '{print tolower($0)}'
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

# Check if the type of install procedure is known
function CheckTypeInstallSupported {
  if [[ $# -ne 1 ]]; then
    echo "ERROR in CheckTypeInstallSupported: Wrong number of arguments"
    exit 1
  fi

  if [ ! $1 == "all" ] && [ ! $1 == "required" ]; then
    echo "ERROR: This type of install operation (${1}) is not supported"
    echo "Supported install types are are: "
    echo "- all: install required and optional packages;"
    echo "- required: install only the required packages."
  fi
}

#
# Arguments:
#   $1 installation type ("all" or "required")
#   $2 path to biodynamo installation src folder (util/installation)
#   $3 OS identifier e.g. ubuntu-16.04 (see DetectOs)
function CompileListOfPackages {
  local BDM_INSTALL_SRC="$2"
  local LOCAL_OS=$3

  # The list of packages on Ubuntu 18.04 and 20.04 are identical to Ubuntu 16.04
  if [ $LOCAL_OS == "ubuntu-18.04" ] || [ $LOCAL_OS == "ubuntu-20.04" ]; then
    LOCAL_OS="ubuntu-16.04"
  fi

  local BDM_INSTALL_OS_SRC=$BDM_INSTALL_SRC/$LOCAL_OS

  BDM_PKG_LIST=""
  if [ $1 == "all" ]; then
    BDM_PKG_LIST="${BDM_INSTALL_OS_SRC}/package_list*"
  else
    BDM_PKG_LIST="${BDM_INSTALL_OS_SRC}/package_list_required"
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
  cmake $BDM_CMAKE_FLAGS ..
  make -j$(CPUCount) && make install
}

# Return absolute path.
# If absolute path is given as parameter it is returned as is. Otherwise it gets converted.
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

  local BDM_PROJECT_DIR="$1"
  shift
  local BDM_SCRIPT_FILE="$1"
  shift
  local BDM_LOCAL_OS=$(DetectOs)
  local BDM_SCRIPT_ARGUMENTS=$@
  local BDM_INSTALL_SRC=$BDM_PROJECT_DIR/util/installation

  # detect os
  local BDM_INSTALL_OS_SRC=$BDM_INSTALL_SRC/$BDM_LOCAL_OS

  # check if this system is supported
  CheckOsSupported $BDM_INSTALL_SRC $BDM_LOCAL_OS

  # check if script exists for the detected OS
  local BDM_SCRIPTPATH=$BDM_INSTALL_OS_SRC/$BDM_SCRIPT_FILE
  if [ ! -f $BDM_SCRIPTPATH ]; then
    echo "ERROR in CallOSSpecificScript: $BDM_SCRIPTPATH does not exist"
    exit 1
  fi

  $BDM_SCRIPTPATH $BDM_SCRIPT_ARGUMENTS
}

# Return the zshrc file based on whether or not ZDOTDIR is defined
function ZshrcFile {
  local zshrc_file="${HOME}/.zshrc"
  if [ -f "$zshrc_file" ]; then true
  elif [ -f "${ZDOTDIR}/.zshrc" ]; then
    zshrc_file="${ZDOTDIR}/.zshrc"
  else
    zshrc_file="<zshrc-file>"
  fi
  echo $zshrc_file
}

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

# Print a warning and list of of offending shell config files
# if there are any naive cases of 'source|. .../thisbdm.sh'
function WarnIfSourceThisbdmInShellConfigs {
  # collect shell config files
  local config_files=("$HOME/.profile")
  if ! [ command -v bash &> /dev/null ]; then
    config_files+=("$HOME/.bashrc" "$HOME/.bash_profile")
  fi
  if ! [ command -v zsh &> /dev/null ]; then
    config_files+=("$HOME/.zshrc" "$HOME/.zshenv" "$HOME/.zprofile")
  fi
  if ! [ command -v fish &> /dev/null ]; then
    config_files+=("$(fish -c 'echo $__fish_config_dir/config.fish')")
  fi
  
  local source_pattern='s/^\s*(\.\s+|source).*thisbdm\.(fish|sh)\s*(#IGNORE)?$/\3/mg;t;d'
  local nr_matches='0'
  local source_occurrences=()
  # check if any config files naively source thisbdm
  for f in ${config_files[@]}; do
    if [ -f "$f" ]; then
      # one may append #IGNORE if match is a false positive, or they really want to keep that line
      nr_matches=$(sed -E "$source_pattern" $f | grep -v "#IGNORE" | wc -l)
      if [ "$nr_matches" -gt '0' ]; then
        source_occurrences+=("$f")
      fi
    fi
  done

  if [ "${#source_occurrences[@]}" -gt '0' ]; then
    EchoWarning "WARN: You may have automatically sourced thisbdm in the following file(s):"
    for f in ${source_occurrences[@]}; do
      EchoWarning "- $f"
    done
    EchoWarning "Please check as this is not advised."
    EchoWarning "If a matching line is a false positive, append ' #IGNORE' to its end."
  fi
}

# Print message that tells the user to reload their shell and source the environment.
# Arguments:
#   $1 biodynamo install directory
function EchoFinishInstallation {
  if [[ $# -ne 1 ]]; then
    echo "ERROR in EchoFinishInstallation: Wrong number of arguments"
    exit 1
  fi

  local addToConfigStr="   ${BDM_ECHO_UNDERLINE}echo \"alias thisbdm='source $1/bin/thisbdm"
  local setQuietEnvVarStr

  EchoInfo "Before running BioDynaMo execute:"
  case $SHELL in
    *bash | *zsh)
      EchoNewStep "   ${BDM_ECHO_UNDERLINE}source $1/bin/thisbdm.sh"

      case $SHELL in
        *bash) addToConfigStr="$addToConfigStr.sh'\" >> $(BashrcFile)" ;;
        *zsh)  addToConfigStr="$addToConfigStr.sh'\" >> $(ZshrcFile)" ;;
      esac
      setQuietEnvVarStr="'export BDM_THISBDM_QUIET=true'"
    ;;
    *fish)
      EchoNewStep "   ${BDM_ECHO_UNDERLINE}source $1/bin/thisbdm.fish"
      addToConfigStr="$addToConfigStr.fish'\" >> (fish -c 'echo "'$__fish_config_dir'"/config.fish')"
      setQuietEnvVarStr="'set -gx BDM_THISBDM_QUIET true'"
    ;;
    *)
      EchoNewStep "   ${BDM_ECHO_UNDERLINE}source $1/bin/thisbdm.{sh|fish}"
      echo
      EchoWarning "WARN: Your login shell appears to be '$SHELL'."
      EchoWarning "BioDynaMo currently supports the following shells:"
      EchoWarning "   ${BDM_ECHO_UNDERLINE}bash, zsh, and fish."
      return
    ;;
  esac

  EchoInfo "For added convenience, run (in your terminal):"
  EchoNewStep "$addToConfigStr"
  EchoInfo "to be able to quickly source thisbdm in your shell session."
  echo
  EchoInfo "${setQuietEnvVarStr} will disable the prompt indicator, and silence non-critical output."
  echo
  EchoNewStep "NOTE: Your login shell appears to be '$SHELL'."
  EchoNewStep "The instructions above are for this shell."
  EchoNewStep "For other shells, or for more information, see:"
  EchoNewStep "   ${BDM_ECHO_UNDERLINE}https://biodynamo.org/docs/userguide/first_steps/"
  # Transition away from telling users to add problematic automatic sourcing of thisbdm
  echo
  WarnIfSourceThisbdmInShellConfigs
}

# This function prompts the user for the biodynamo installation directory
# providing an option to accept a default.
function SelectInstallDir {
  if [[ $# -ne 0 ]]; then
    echo "ERROR in SelectInstallDir: Wrong number of arguments"
    exit 1
  fi

  local DEFAULT=~/biodynamo
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

# This function has a zero exit code if the version is below the required
# version. This function can be used inside an if statement:
#    if $(versionLessThan "$(gcc -dumpversion)" "5.4.0"); then
#       echo "gcc version less than 5.4.0"
#    fi
# Arguments:
#   $1 actual version
#   $2 required version
function VersionLessThan {
  local VERSION=$1
  local REQUIRED=$2
  [ "$(printf '%s\n' "$REQUIRED" "$VERSION" | sort -V | head -n1)" != "$REQUIRED" ]
}

function GetVersionFromString() {
  echo $($1 --version | perl -pe 'if(($v)=/([0-9]+([.][0-9]+)+)/){print"$v\n";exit}$_=""')
}

# This function checks if CMake is installed on the system and if the version
# meets the minimum required version ($1). On some systems the alias `cmake3`
# is used to point to CMake v3.X.X, so we set that as the binary for building
# BioDynaMo during installation.
function CheckCMakeVersion {
  if [ -x "$(command -v cmake3)" ]; then
    CMAKE_BINARY=cmake3
  elif [ -x "$(command -v cmake)" ]; then
    CMAKE_BINARY=cmake
  else
    echo "Error: CMake not found on this system. Please install CMake with version $1 or higher."
    exit 1
  fi
  VER=`GetVersionFromString $CMAKE_BINARY`
  if $(VersionLessThan $VER $1); then
    echo "Error: CMake version must be at least v$1"
    exit 1
  fi
  export CMAKE_BINARY=$CMAKE_BINARY
}
