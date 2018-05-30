#!/bin/bash
# This file contains functiom=ns required in various install scripts.
# (Thus reducing code duplication)

# This function checks if a script is run with sudo rights. If not, it will exit
# with -1 return code
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

# This function prompts a user with a yes, no questions. If answered with `yes`
# it executes the given function, otherwise it calls exit
# Arguments:
#   $1 Message displayed to the user. The function appends "(y/n)" to the message
#   $2 The function that should be executed if the user replies with `yes`
function PromptUser {
  if [[ $# -ne 2 ]]; then
    echo "ERROR in PromptUser: Wrong number of arguments"
    exit 1
  fi

  # https://stackoverflow.com/questions/226703/how-do-i-prompt-for-yes-no-cancel-input-in-a-linux-shell-script
  while true; do
    read -p "$1 (y/n) " yn
    case $yn in
      [Yy]* ) $2; exit;;
      [Nn]* ) exit 1;;
          * ) echo "Please answer yes or no.";;
    esac
  done
}

# Detects the linux flavour using `lsb_release`
# Returns a lower case version of `distributor-release`
function DetectLinuxFlavour {
  DISTRIBUTOR=$(lsb_release -si)
  RELEASE=$(lsb_release -sr)
  OS="${DISTRIBUTOR}-${RELEASE}"
  echo $OS | awk '{print tolower($0)}'
}

# Function that detects the OS
# Returns linux flavour or macos.
# This function prints an error and exits if is not linux or macos
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
# prints an error message, a list of supported systems; and exits the script
# Arguments:
#   $1 path to biodynamo installation src folder (cmake/installation)
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

# Download a tar file extracts the contents into the destination folder
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

  rm $TMP_DEST || true
  sudo mkdir -p $DEST
  if [ $BDM_LOCAL_CERNBOX ]  && [[ "$URL" = /* ]]; then
    sudo tar -xzf $URL --strip=$STRIP_COMP -C $DEST
  else
    wget --progress=dot:giga -O $TMP_DEST $URL
    sudo tar -xzf $TMP_DEST --strip=$STRIP_COMP -C $DEST
    rm $TMP_DEST
  fi
}

# Returns a CERNBox download link
# If the environment variable BDM_LOCAL_CERNBOX is set to a local copy of CERNBox,
# this function returns a local path to the requested file
# Arguments:
#   $1 directory
#   $2 file
function CernboxLink {
  if [[ $# -ne 2 ]]; then
    echo "ERROR in CernboxLink: Wrong number of arguments"
    exit 1
  fi
  if [ ! $BDM_LOCAL_CERNBOX ]; then
    echo "https://cernbox.cern.ch/index.php/s/GjoGUe6HGndEgmP/download?path=${1}&files=${2}"
  else
    echo "${BDM_LOCAL_CERNBOX}/${1}/${2}"
  fi
}

# Download a tar file from CERNBox and extracts the contents into the
# destination folder
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

  URL=$(CernboxLink $DIR $FILE)

  DownloadTarAndExtract $URL $DEST
}

# Returns the number of CPU cores including hyperthreads
function CPUCount {
  grep -c ^processor /proc/cpuinfo
}

# Updates the use_biodynamo variable in the bashrc file
# Arguments:
#   $1 path to biodynamo environment file
function UpdateSourceBdmVariable {
  if [[ $# -ne 1 ]]; then
    echo "ERROR in UpdateSourceBdmVariable: Wrong number of arguments"
    exit 1
  fi

  local BDM_BASHRC=$(BashrcFile)
  local TMP_BASHRC=$(BashrcFile)_$RANDOM
  touch $BDM_BASHRC
  echo "use_biodynamo='source ${1}'" > $TMP_BASHRC
  cat $BDM_BASHRC | grep -v "use_biodynamo" >> $TMP_BASHRC
  mv $TMP_BASHRC $BDM_BASHRC
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
    sudo rm -rf $BUILD_DIR/*
  else
    mkdir -p $BUILD_DIR
  fi
  cd $BUILD_DIR
  echo "CMAKEFLAGS $BDM_CMAKE_FLAGS"
  cmake $BDM_CMAKE_FLAGS .. && make -j$(CPUCount) && sudo make install
}

# Return absolute path
# If absolute path is given as paramter it is returned as is. Otherwise it gets
# converted
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

  local BDM_INSTALL_SRC=$BDM_PROJECT_DIR/cmake/installation

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

# Print message that tells the user to reload the bash and call $use_biodynamo
function EchoFinishThisStep {
  local BDM_BASHRC=$(BashrcFile)
  echo "To complete this step please restart your shell or execute"
  echo "    . $BDM_BASHRC"
  echo "In every terminal you want to build or use BioDynamo execute:"
  echo "    \$use_biodynamo"
}
