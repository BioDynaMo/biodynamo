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
  #brew install cloc
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
  export PATH=$LLVMDIR/bin:"`pwd`/cmake-3.6.1-Darwin-x86_64/CMake.app/Contents/bin":$PATH:
fi

if [ "$TRAVIS_OS_NAME" = "linux" ]; then
  sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test  # gcc-5
  wget -O - http://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
  sudo apt-add-repository -y "deb http://apt.llvm.org/trusty/ llvm-toolchain-trusty-3.9 main"
  sudo apt-get update
  sudo apt-get -y install gcc-5 g++-5
  sudo apt-get -y install valgrind
  sudo apt-get -y install doxygen
  sudo apt-get -y install cloc
  sudo apt-get -y install clang-3.9 clang-format-3.9 clang-tidy-3.9
  export CC=gcc-5
  export CXX=g++-5
fi

# install ROOT
cd
if [ "$TRAVIS_OS_NAME" = "linux" ]; then
  wget https://root.cern.ch/download/root_v6.06.04.Linux-ubuntu14-x86_64-gcc4.8.tar.gz 2> /dev/null
  tar zxf root_v6.06.04.Linux-ubuntu14-x86_64-gcc4.8.tar.gz > /dev/null
else
  # write progress to terminal to prevent termination by travis if it takes longer than 10 min
  wget --progress=dot:giga https://root.cern.ch/download/root_v6.06.00.macosx64-10.9-clang60.tar.gz
  tar zxf root_v6.06.00.macosx64-10.9-clang60.tar.gz > /dev/null
fi
cd root
. bin/thisroot.sh

# output compiler information
echo ${CXX}
${CXX} --version
${CXX} -v

cd $biod

# run following commands only on Linux
if [ "$TRAVIS_OS_NAME" = "linux" ]; then
  cloc .
  # required by housekeeping/run-clang-tidy.sh due to issues on Linux and g++ builds
  # clang-tidy did not find omp.h in this configuration
  mkdir /tmp/bdm_omp
  cp /usr/lib/gcc/x86_64-linux-gnu/4.8/include/omp.h /tmp/bdm_omp/
fi

# add master branch
# https://github.com/travis-ci/travis-ci/issues/6069
git remote set-branches --add origin master

# build biodynamo and run tests
mkdir build
cd build
cmake $test_valgrind ..
make check-submission
