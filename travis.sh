#!/bin/bash

set -e -x

echo ${TRAVIS_OS_NAME}
BDM_DIR=`pwd`

# update and install packages
if [ "$TRAVIS_OS_NAME" = "osx" ]; then
  sw_vers
  osx_vers=`sw_vers -productVersion | cut -d . -f1 -f2`
  brew update >& /dev/null
  brew install doxygen
  brew install valgrind
  brew install cloc
  # get clang 5.0
  brew install llvm
  export LLVMDIR="/usr/local/opt/llvm"
  export CC=$LLVMDIR/bin/clang
  export CXX=$LLVMDIR/bin/clang++
  export CXXFLAGS=-I$LLVMDIR/include
  export LDFLAGS=-L$LLVMDIR/lib
  export DYLD_LIBRARY_PATH=$LLVMDIR/lib:$DYLD_LIBRARY_PATH
  # get latest cmake
  brew upgrade cmake
  export PATH=$LLVMDIR/bin:$PATH
  # copy the omp.h file to our CMAKE_PREFIX_PATH
  sudo mkdir -p /usr/local/Cellar/biodynamo
  OMP_V=`/usr/local/opt/llvm/bin/llvm-config --version`
  sudo cp -f /usr/local/opt/llvm/lib/clang/$OMP_V/include/omp.h /usr/local/Cellar/biodynamo
fi

if [ "$TRAVIS_OS_NAME" = "linux" ]; then
  sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test  # gcc-5
  wget -O - http://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
  sudo apt-add-repository -y "deb http://apt.llvm.org/trusty/ llvm-toolchain-trusty-3.9 main"
  sudo apt-get update
  sudo apt-get -y install openmpi-bin libopenmpi-dev
  sudo apt-get -y install freeglut3-dev
  sudo apt-get -y install gcc-5 g++-5
  sudo apt-get -y install valgrind
  sudo apt-get -y install doxygen
  sudo apt-get -y install cloc
  sudo apt-get -y install libiomp-dev
  sudo apt-get -y install clang-3.9 clang-format-3.9 clang-tidy-3.9
  export CC=gcc-5
  export CXX=g++-5

  # Need a CMake version >= 3.3 for VTK (Catalyst)
  cd
  wget https://cmake.org/files/v3.6/cmake-3.6.3-Linux-x86_64.tar.gz
  tar -xzf cmake-3.6.3-Linux-x86_64.tar.gz
  sudo rm -f /usr/bin/cmake
  sudo ln -s `pwd`/cmake-3.6.3-Linux-x86_64/bin/cmake /usr/bin/cmake

  # needed for Catalyst
  sudo ln -s /usr/lib/libmpi.so /usr/local/lib/libmpi.so
  sudo ln -s /usr/lib/libmpi.so /usr/local/lib/libmpi.so.12
fi

# install third_party applications
THIRD_PARTY_DIR=/opt/biodynamo/third_party
sudo mkdir -p $THIRD_PARTY_DIR
#   download tars to tmp directory
cd /tmp
BDM_ENVIRONMENT_FILE=/opt/biodynamo/biodynamo_dev.env

if [ "$TRAVIS_OS_NAME" = "linux" ]; then
  # install ROOT
  wget --progress=dot:giga -O root_v6.11.01_Linux-ubuntu14-x86_64-gcc5.4_263508429d.tar.gz "https://cernbox.cern.ch/index.php/s/1ekMudArMqFhcXt/download?path=%2F&files=root_v6.11.01_Linux-ubuntu14-x86_64-gcc5.4_263508429d.tar.gz"
  sudo tar zxf "root_v6.11.01_Linux-ubuntu14-x86_64-gcc5.4_263508429d.tar.gz" -C $THIRD_PARTY_DIR

  # install qt
  wget -O Qt5.9.1_ubuntu16_gcc5.4.tar.gz "https://cernbox.cern.ch/index.php/s/1ekMudArMqFhcXt/download?path=%2F&files=Qt5.9.1_ubuntu16_gcc5.4.tar.gz"
  sudo mkdir -p $THIRD_PARTY_DIR/qt
  sudo tar -xzf Qt5.9.1_ubuntu16_gcc5.4.tar.gz -C $THIRD_PARTY_DIR/qt

  # install paraview
  wget -O paraview-5.4_ubuntu14_gcc5.4_openmpi.tar.gz "https://cernbox.cern.ch/index.php/s/1ekMudArMqFhcXt/download?path=%2F&files=paraview-5.4_ubuntu14_gcc5.4_openmpi.tar.gz"
  sudo mkdir -p $THIRD_PARTY_DIR/paraview
  sudo tar -xzf paraview-5.4_ubuntu14_gcc5.4_openmpi.tar.gz -C $THIRD_PARTY_DIR/paraview

  # copy environment script
  sudo cp $BDM_DIR/cmake/biodynamo_linux_dev.env $BDM_ENVIRONMENT_FILE
else
  # install ROOT
  wget --progress=dot:giga -O root_v6.11.01_macos64_LLVM-Clang-5.0_263508429d.tar.gz "https://cernbox.cern.ch/index.php/s/1ekMudArMqFhcXt/download?path=%2F&files=root_v6.11.01_macos64_LLVM-Clang-5.0_263508429d.tar.gz"
  sudo sudo tar zxf "root_v6.11.01_macos64_LLVM-Clang-5.0_263508429d.tar.gz" -C $THIRD_PARTY_DIR

  # install qt and python3
  brew install qt
  brew upgrade python3

  # install paraview
  wget -O paraview-5.4_macos64_llvm-5.0.tar.gz "https://cernbox.cern.ch/index.php/s/1ekMudArMqFhcXt/download?path=%2F&files=paraview-5.4_macos64_llvm-5.0.tar.gz"
  sudo mkdir -p $THIRD_PARTY_DIR/paraview
  sudo tar -xzf paraview-5.4_macos64_llvm-5.0.tar.gz -C $THIRD_PARTY_DIR/paraview

  # copy environment script
  sudo cp $BDM_DIR/cmake/biodynamo_macos_dev.env $BDM_ENVIRONMENT_FILE
fi

# source environment file
. $BDM_ENVIRONMENT_FILE

# output compiler information
echo ${CXX}
${CXX} --version
${CXX} -v

cd $BDM_DIR

cloc .
# add master branch
# https://github.com/travis-ci/travis-ci/issues/6069
git remote set-branches --add origin master

# build biodynamo and run tests
mkdir build
cd build
mkdir install

cmake -DCMAKE_BUILD_TYPE=Release ..
make -j2
# install build is required by e.g. makefile_project integration test
sudo make install
make check-submission
