#!/bin/bash

if ! cmake_loc="$(type -p cmake)" || [ -z "$cmake_loc" ]; then
  wget https://cmake.org/files/v3.6/cmake-3.6.3-Linux-x86_64.tar.gz
  tar -xzvf cmake-3.6.3-Linux-x86_64.tar.gz -C /usr/local/bin cmake-3.6.3-Linux-x86_64/bin --strip 2
fi

sudo apt-get install mpich
sudo apt-get install freeglut3-dev

ln -s /usr/lib/libmpi.so /usr/local/lib/libmpi.so
ln -s /usr/lib/libmpi.so /usr/local/lib/libmpi.so.12

wget "https://cernbox.cern.ch/index.php/s/BbFptgxo2K565IS/download?path=%2F&files=paraview-catalyst-5.4.0_ubuntu16_gcc5.4.tar.gz"
tar zxf paraview-catalyst-5.4.0_ubuntu16_gcc5.4.tar.gz
export ParaView_DIR="`pwd`/paraview-catalyst-5.4.0_ubuntu16_gcc5.4/lib/cmake/paraview-5.4"

export PYTHONPATH=/home/paraview-catalyst-5.4.0_ubuntu16_gcc5.4/lib/paraview-5.4/site-packages
export PYTHONPATH=$PYTHONPATH:/home/paraview-catalyst-5.4.0_ubuntu16_gcc5.4/lib/paraview-5.4/site-packages/vtk/