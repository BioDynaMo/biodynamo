#!/bin/bash
# This script installs the required packages on ubuntu 16.04 as outlined in the
# main README.md

function Install {
  THIRD_PARTY_DIR=$INSTALL_DIR/third_party

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

  wget -O paraview-5.4_ubuntu14_gcc5.4_openmpi.tar.gz "https://cernbox.cern.ch/index.php/s/BbFptgxo2K565IS/download?path=%2F&files=paraview-5.4_ubuntu14_gcc5.4_openmpi.tar.gz"
  mkdir -p $THIRD_PARTY_DIR/paraview
  tar -xzf paraview-5.4_ubuntu14_gcc5.4_openmpi.tar.gz -C $THIRD_PARTY_DIR/paraview

  wget -O Qt5.6.2_ubuntu16_gcc5.4.tar.gz "https://cernbox.cern.ch/index.php/s/BbFptgxo2K565IS/download?path=%2F&files=Qt5.6.2_ubuntu16_gcc5.4.tar.gz"
  mkdir -p $THIRD_PARTY_DIR/qt
  tar -xzf Qt5.6.2_ubuntu16_gcc5.4.tar.gz -C $THIRD_PARTY_DIR/qt

  touch bdm_dependencies.sh

  echo 'export CC=gcc-5' >> bdm-dependencies.sh
  echo 'export CXX=g++-5' >> bdm-dependencies.sh

  echo "export ParaView_DIR=$THIRD_PARTY_DIR/paraview/lib/cmake/paraview-5.4" >> ~/.bashrc
  echo "export Qt5_DIR=$THIRD_PARTY_DIR/qt/lib/cmake/Qt5" >> bdm-dependencies.sh
  echo "export LD_LIBRARY_PATH=$THIRD_PARTY_DIR/qt/lib" >> bdm-dependencies.sh

  echo "export PYTHONPATH=$THIRD_PARTY_DIR/paraview/lib/paraview-5.4/site-packages:$THIRD_PARTY_DIR/paraview/lib/paraview-5.4/site-packages/vtk" >> bdm-dependencies.sh

  echo "source `pwd`/bdm-dependencies.sh" >> ~/.bashrc
  exec bash

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
