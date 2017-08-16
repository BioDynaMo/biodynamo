#!/bin/bash

sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
wget -O - http://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
sudo apt-add-repository -y "deb http://apt.llvm.org/trusty/ llvm-toolchain-trusty-3.9 main"
sudo apt-get update
sudo apt-get -y install gcc-5 g++-5

# update cmake
wget https://cmake.org/files/v3.6/cmake-3.6.3-Linux-x86_64.tar.gz
tar -xzf cmake-3.6.3-Linux-x86_64.tar.gz
sudo rm /usr/bin/cmake
sudo ln -s `pwd`/cmake-3.6.3-Linux-x86_64/bin/cmake /usr/bin/cmake

cd /root
git clone https://github.com/breitwieserCern/root.git

mkdir install
mkdir install/root

mkdir build
cd build
cmake -DCMAKE_C_COMPILER=gcc-5 -DCMAKE_CXX_COMPILER=g++-5 -DCMAKE_INSTALL_PREFIX=/root/install/root -Dcxx14=on ../root/
make -j4 install

cd ../install
tar -zcf root.tar.gz root
