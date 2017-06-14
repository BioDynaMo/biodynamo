#!/bin/bash
# This script installs the required packages on ubuntu 16.04 as outlined in the
# main README.md

function Install {
  echo "Start installation of prerequisites..."
  apt-get update

  # install `apt-add-repository and wget if not already installed
  # (missing on docker image)
  apt-get install -y software-properties-common wget

  # add repository for clang-3.9
  wget -O - http://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add -
  apt-add-repository -y "deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-3.9 main"
  apt-get update

  # install packages
  apt-get -y install git g++ cmake valgrind python lcov
  apt-get -y install clang-3.9 clang-format-3.9 clang-tidy-3.9 libomp-dev
  apt-get -y install doxygen graphviz

  # # install ROOT
  # mkdir /opt/ROOT
  # cd /opt/ROOT
  # wget https://root.cern.ch/download/root_v6.06.04.Linux-ubuntu14-x86_64-gcc4.8.tar.gz 2> /dev/null
  # tar zxf root_v6.06.04.Linux-ubuntu14-x86_64-gcc4.8.tar.gz > /dev/null
  # # add this to ~/.bashrc
  # if [ "$(cat ~/.bashrc | grep ". /opt/ROOT/bin/thisroot.sh" | wc -l)" != "0" ]; then
  #   echo ". /opt/ROOT/bin/thisroot.sh" >> ~/.bashrc
  # fi
}

if [ "$(whoami)" != "root" ]; then
  echo "Error: This script requires root access. Exiting now."
  exit;
fi

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
