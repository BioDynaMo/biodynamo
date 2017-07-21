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
  add-apt-repository -y ppa:ubuntu-toolchain-r/test  # gcc-5
  apt-add-repository -y "deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-3.9 main"
  apt-get update

  # todo: check for install cmake version
  wget https://cmake.org/files/v3.6/cmake-3.6.3-Linux-x86_64.tar.gz
  tar -xzf cmake-3.6.3-Linux-x86_64.tar.gz
  rm /usr/bin/cmake
  ln -s `pwd`/cmake-3.6.3-Linux-x86_64/bin/cmake /usr/bin/cmake

  # install packages
  apt-get -y install libopenmpi-dev openmpi-bin
  apt-get -y install freeglut3-dev
  apt-get -y install git valgrind python python2.7-dev lcov
  apt-get -y install gcc-5 g++-5
  apt-get -y install clang-3.9 clang-format-3.9 clang-tidy-3.9 libomp-dev
  apt-get -y install doxygen graphviz

  # needed for Catalyst
  ln -s /usr/lib/libmpi.so /usr/local/lib/libmpi.so
  ln -s /usr/lib/libmpi.so /usr/local/lib/libmpi.so.12

  wget -O paraview-catalyst-5.4.0_ubuntu14_gcc5.4.tar.gz "https://cernbox.cern.ch/index.php/s/BbFptgxo2K565IS/download?path=%2F&files=paraview-catalyst-5.4.0_ubuntu14_gcc5.4.tar.gz"
  tar zxf paraview-catalyst-5.4.0_ubuntu14_gcc5.4.tar.gz

  echo "================"
  echo "Set the following environmental variables in your login shell profile file (such as .bashrc):"
  echo "export ParaView_DIR=`pwd`/paraview-catalyst-5.4.0_ubuntu14_gcc5.4/lib/cmake/paraview-5.4"
  echo "export PYTHONPATH=$ParaView_DIR/../../paraview-5.4/site-packages"
  echo "export PYTHONPATH=$PYTHONPATH:$ParaView_DIR/../../paraview-5.4/site-packages/vtk"
  echo "================"

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
