#!/bin/bash
# This script installs the required packages on ubuntu 16.04 as outlined in the
# main README.md

function Install {
  THIRD_PARTY_DIR=$INSTALL_DIR/third_party

  echo "Start installation of prerequisites..."
  apt-get update

  # Remove everything in ${THIRD_PARTY_DIR} if it exists already.
  # Might contain outdated dependencies
  if [ -d "${THIRD_PARTY_DIR}" ]; then
    rm -rf "${THIRD_PARTY_DIR}/*"
  else
    mkdir -p $THIRD_PARTY_DIR
  fi

  # install `apt-add-repository and wget if not already installed
  # (missing on docker image)
  apt-get install -y software-properties-common wget

  # add repository for clang-3.9
  wget -O - http://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add -
  apt-add-repository -y "deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-3.9 main"
  apt-get update

  # install packages
  apt-get -y install libopenmpi-dev openmpi-bin
  apt-get -y install freeglut3-dev
  apt-get -y install git valgrind python python2.7-dev lcov
  apt-get -y install gcc-5 g++-5
  apt-get -y install clang-3.9 clang-format-3.9 clang-tidy-3.9 libomp-dev
  apt-get -y install doxygen graphviz

  # install ROOT
  wget -O /tmp/root_dict_patch.Linux-ubuntu16-x86_64-gcc5.4.tar.gz "https://cernbox.cern.ch/index.php/s/BbFptgxo2K565IS/download?path=%2F&files=root_dict_patch.Linux-ubuntu16-x86_64-gcc5.4.tar.gz"
  tar -xzf /tmp/root_dict_patch.Linux-ubuntu16-x86_64-gcc5.4.tar.gz -C $THIRD_PARTY_DIR

  # create environment script
  BDM_ENVIRONMENT_FILE=${THIRD_PARTY_DIR}/bdm_environment.sh
  touch ${BDM_ENVIRONMENT_FILE}

  echo ". ${THIRD_PARTY_DIR}/root/bin/thisroot.sh" >> ${BDM_ENVIRONMENT_FILE}

  # add to ~/.bashrc
  if [ "$(cat ~/.bashrc | grep ". ${BDM_ENVIRONMENT_FILE}" | wc -l)" == "0" ]; then
    echo "Adding \". ${BDM_ENVIRONMENT_FILE}\" to .bashrc"
    echo ". ${BDM_ENVIRONMENT_FILE}" >> ~/.bashrc
    . ~/.bashrc
  fi
}

if [ "$(whoami)" != "root" ]; then
  echo "Error: This script requires root access. Exiting now."
  exit;
fi

# prompts user for installation directory
while true; do
  read -p "The default installation directory is /opt/biodynamo.
Do you want to change the installation directory? (y/n) " yn
  case $yn in
    [Yy]* ) INSTALL_DIR="$(zenity --file-selection --directory)"; break;;
    [Nn]* ) INSTALL_DIR=/opt/biodynamo; break;;
        * ) echo "Please answer yes or no.";;
  esac
done

echo ""

# ask user if she really wants to perform this changes
# https://stackoverflow.com/questions/226703/how-do-i-prompt-for-yes-no-cancel-input-in-a-linux-shell-script
while true; do
  read -p "This script adds the following package repository:
'deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-3.9 main'
and installs the packages mentioned in the main README.md.
Do you want to continue? (y/n) " yn
  case $yn in
    [Yy]* ) Install; break;;
    [Nn]* ) exit;;
        * ) echo "Please answer yes or no.";;
  esac
done
