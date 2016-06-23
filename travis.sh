#!/bin/bash

echo ${TRAVIS_OS_NAME}

# update and install packges
if [ "$TRAVIS_OS_NAME" = "osx" ]; then
  brew update
  brew install doxygen
  brew outdated gcc || brew upgrade gcc
fi

if [ "$TRAVIS_OS_NAME" = "linux" ]; then
  sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
  sudo add-apt-repository -y ppa:george-edison55/precise-backports
  sudo apt-get update
  sudo apt-get -y install gcc-5 g++-5 cmake cmake-data valgrind
fi


# link to newest compiler
if [ "$TRAVIS_OS_NAME" = "linux" ] && [ "$CXX" = "g++" ]; then export CXX="g++-5" CC="gcc-5"; fi
if [ "$TRAVIS_OS_NAME" = "linux" ] && [ "$CXX" = "clang++" ]; then export CXX="clang++-3.7" CC="clang-3.7"; fi
if [ "$TRAVIS_OS_NAME" = "osx" ] && [ "$CXX" = "g++" ]; then export CXX="/usr/local/Cellar/gcc/5.3.0/bin/g++-5" CC="/usr/local/Cellar/gcc/5.3.0/bin/gcc-5"; fi

# output compiler information
echo ${CXX}
${CXX} --version
${CXX} -v

# build and run tests
mkdir build
cd build
cmake .. && make
make check