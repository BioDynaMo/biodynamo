#!/bin/bash
# Argument
#   $1 root commit_id

add-apt-repository -y ppa:ubuntu-toolchain-r/test
wget -O - http://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add -
apt-add-repository -y "deb http://apt.llvm.org/trusty/ llvm-toolchain-trusty-3.9 main"
apt-get update
apt-get -y install gcc-5 g++-5
# only for ubuntu image
apt-get -y install wget git make
#  root required packages
apt-get -y install git dpkg-dev cmake g++ gcc binutils libx11-dev libxpm-dev \
  libxft-dev libxext-dev
#  root optional packages
apt-get -y install gfortran libssl-dev libpcre3-dev \
  xlibmesa-glu-dev libglew1.5-dev libftgl-dev \
  libmysqlclient-dev libfftw3-dev libcfitsio-dev \
  graphviz-dev libavahi-compat-libdnssd-dev \
  libldap2-dev python-dev libxml2-dev libkrb5-dev \
  libgsl0-dev libqt4-dev

# update cmake
wget https://cmake.org/files/v3.6/cmake-3.6.3-Linux-x86_64.tar.gz
tar -xzf cmake-3.6.3-Linux-x86_64.tar.gz
rm /usr/bin/cmake
ln -s `pwd`/cmake-3.6.3-Linux-x86_64/bin/cmake /usr/bin/cmake

cd /root
git clone https://github.com/root-project/root.git

cd root
git checkout $1
git status
cd ..

mkdir install
mkdir install/root

mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=gcc-5 -DCMAKE_CXX_COMPILER=g++-5 -DCMAKE_INSTALL_PREFIX=/root/install/root -Dcxx14=on ../root/
make -j4 install

cd ../install
tar -zcf root.tar.gz root
