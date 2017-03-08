#!/bin/bash

set -e -x

echo ${TRAVIS_OS_NAME}
biod=`pwd`

# update and install packages
if [ "$TRAVIS_OS_NAME" = "osx" ]; then
  sw_vers
  osx_vers=`sw_vers -productVersion | cut -d . -f1 -f2`
  if [ "$osx_vers" != "10.12" ]; then
     test_valgrind="-Dvalgrind=ON"
  fi
  brew update >& /dev/null
  brew install doxygen
  brew install valgrind
  # get clang 3.9
  wget http://releases.llvm.org/3.9.0/clang+llvm-3.9.0-x86_64-apple-darwin.tar.xz 2> /dev/null
  tar xf clang+llvm-3.9.0-x86_64-apple-darwin.tar.xz > /dev/null
  export LLVMDIR="`pwd`/clang+llvm-3.9.0-x86_64-apple-darwin"
  export CC=$LLVMDIR/bin/clang
  export CXX=$LLVMDIR/bin/clang++
  export CXXFLAGS=-I$LLVMDIR/include
  export LDFLAGS=-L$LLVMDIR/lib
  export DYLD_LIBRARY_PATH=$LLVMDIR/lib:$DYLD_LIBRARY_PATH
  # get latest cmake
  wget https://cmake.org/files/v3.6/cmake-3.6.1-Darwin-x86_64.tar.gz 2> /dev/null
  tar zxf cmake-3.6.1-Darwin-x86_64.tar.gz > /dev/null
  export PATH="`pwd`/cmake-3.6.1-Darwin-x86_64/CMake.app/Contents/bin":$PATH:
fi

if [ "$TRAVIS_OS_NAME" = "linux" ]; then
  #sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
  #sudo add-apt-repository -y ppa:george-edison55/precise-backports
  #sudo apt-get update
  #sudo apt-get -y install gcc-5 g++-5 cmake cmake-data valgrind
  sudo apt-get -y install valgrind
fi

# install ROOT
cd
if [ "$TRAVIS_OS_NAME" = "linux" ]; then
  wget https://root.cern.ch/download/root_v6.06.04.Linux-ubuntu14-x86_64-gcc4.8.tar.gz 2> /dev/null
  tar zxf root_v6.06.04.Linux-ubuntu14-x86_64-gcc4.8.tar.gz > /dev/null
else
  wget https://root.cern.ch/download/root_v6.06.00.macosx64-10.9-clang60.tar.gz 2> /dev/null
  tar zxf root_v6.06.00.macosx64-10.9-clang60.tar.gz > /dev/null
fi
cd root
. bin/thisroot.sh

# output compiler information
echo ${CXX}
${CXX} --version
${CXX} -v

# build biodynamo and run tests
cd $biod
mkdir build
cd build
cmake $test_valgrind .. && make
make check
