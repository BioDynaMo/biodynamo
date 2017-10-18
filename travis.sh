#!/bin/bash

set -e -x

echo ${TRAVIS_OS_NAME}
biod=`pwd`

# update and install packages
if [ "$TRAVIS_OS_NAME" = "osx" ]; then
  sw_vers
  osx_vers=`sw_vers -productVersion | cut -d . -f1 -f2`
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
  sudo rm /usr/bin/cmake
  sudo ln -s `pwd`/cmake-3.6.3-Linux-x86_64/bin/cmake /usr/bin/cmake

  # needed for Catalyst
  sudo ln -s /usr/lib/libmpi.so /usr/local/lib/libmpi.so
  sudo ln -s /usr/lib/libmpi.so /usr/local/lib/libmpi.so.12
fi

# install ROOT
cd
if [ "$TRAVIS_OS_NAME" = "linux" ]; then
  wget --progress=dot:giga -O root_v6.11.01_Linux-ubuntu14-x86_64-gcc5.4_263508429d.tar.gz "https://cernbox.cern.ch/index.php/s/BbFptgxo2K565IS/download?path=%2F&files=root_v6.11.01_Linux-ubuntu14-x86_64-gcc5.4_263508429d.tar.gz"
  tar zxf "root_v6.11.01_Linux-ubuntu14-x86_64-gcc5.4_263508429d.tar.gz" > /dev/null

  wget -O paraview-5.4_ubuntu14_gcc5.4_openmpi.tar.gz "https://cernbox.cern.ch/index.php/s/BbFptgxo2K565IS/download?path=%2F&files=paraview-5.4_ubuntu14_gcc5.4_openmpi.tar.gz"
  sudo mkdir -p /opt/biodynamo/paraview
  sudo tar -xzf paraview-5.4_ubuntu14_gcc5.4_openmpi.tar.gz -C /opt/biodynamo/paraview

  wget -O Qt5.9.1_ubuntu16_gcc5.4.tar.gz "https://cernbox.cern.ch/index.php/s/BbFptgxo2K565IS/download?path=%2F&files=Qt5.9.1_ubuntu16_gcc5.4.tar.gz"
  sudo mkdir -p /opt/biodynamo/qt
  sudo tar -xzf Qt5.9.1_ubuntu16_gcc5.4.tar.gz -C /opt/biodynamo/qt

  export ParaView_DIR=/opt/biodynamo/paraview/lib/cmake/paraview-5.4
  export Qt5_DIR=/opt/biodynamo/qt/lib/cmake/Qt5

  export LD_LIBRARY_PATH=/opt/biodynamo/qt/lib:/usr/lib/openmpi/lib:$LD_LIBRARY_PATH
else
  # write progress to terminal to prevent termination by travis if it takes longer than 10 min
  wget --progress=dot:giga https://root.cern.ch/download/root_v6.10.00.macosx64-10.11-clang80.tar.gz
  tar zxf root_v6.10.00.macosx64-10.11-clang80.tar.gz > /dev/null
fi

# set the Python envars for Catalyst
export PYTHONPATH=$ParaView_DIR/../../paraview-5.4/site-packages
export PYTHONPATH=$PYTHONPATH:$ParaView_DIR/../../paraview-5.4/site-packages/vtk

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

  # add master branch
  # https://github.com/travis-ci/travis-ci/issues/6069
  git remote set-branches --add origin master

  # build biodynamo and run tests
  mkdir build
  cd build
  cmake ..
  make check-submission
fi
