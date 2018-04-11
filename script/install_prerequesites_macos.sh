#!/bin/bash
# This script installs the required packages on ubuntu 16.04 as outlined in the
# main README.md

INSTALL_DIR=/opt/biodynamo

function Install {
  sudo echo "Start installation of prerequisites..."

  THIRD_PARTY_DIR=$INSTALL_DIR/third_party

  # Remove everything in ${THIRD_PARTY_DIR} if it exists already.
  # Might contain outdated dependencies
  if [ -d "${THIRD_PARTY_DIR}" ]; then
    rm -rf "${THIRD_PARTY_DIR}/*"
  else
    sudo mkdir -p $THIRD_PARTY_DIR
  fi

  # install packages
  brew update >& /dev/null
  brew install doxygen
  brew install lcov
  brew install cloc
  brew install llvm
  brew install python
  brew install python3
  brew upgrade cmake
  brew install qt

  export LLVMDIR="/usr/local/opt/llvm"
  export CC=$LLVMDIR/bin/clang
  export CXX=$LLVMDIR/bin/clang++
  export CXXFLAGS=-I$LLVMDIR/include
  export LDFLAGS=-L$LLVMDIR/lib
  export DYLD_LIBRARY_PATH=$LLVMDIR/lib:$DYLD_LIBRARY_PATH
  export PATH=$LLVMDIR/bin:$PATH

  # copy the omp.h file to our CMAKE_PREFIX_PATH
  sudo mkdir -p /usr/local/Cellar/biodynamo
  OMP_V=`/usr/local/opt/llvm/bin/llvm-config --version`
  sudo cp -f $LLVMDIR/lib/clang/$OMP_V/include/omp.h /usr/local/Cellar/biodynamo

  # install ROOT
  wget --progress=dot:giga -O root_v6.11.01_macos64_LLVM-Clang-5.0_263508429d.tar.gz "https://cernbox.cern.ch/index.php/s/1ekMudArMqFhcXt/download?path=%2F&files=root_v6.11.01_macos64_LLVM-Clang-5.0_263508429d.tar.gz"
  sudo tar -xzf root_v6.11.01_macos64_LLVM-Clang-5.0_263508429d.tar.gz -C $THIRD_PARTY_DIR

  # copy environment script
  #   get path of this script
  pushd `dirname $0` > /dev/null
  SCRIPTPATH=`pwd`
  popd > /dev/null
  BDM_ENVIRONMENT_FILE=/opt/biodynamo/biodynamo_dev.env
  sudo cp $SCRIPTPATH/../cmake/biodynamo_macos_dev.env $BDM_ENVIRONMENT_FILE

  # install ParaView
  wget -O paraview-5.4_macos64_llvm-5.0.tar.gz "https://cernbox.cern.ch/index.php/s/1ekMudArMqFhcXt/download?path=%2F&files=paraview-5.4_macos64_llvm-5.0.tar.gz"
  sudo mkdir -p $THIRD_PARTY_DIR/paraview
  sudo tar -xzf paraview-5.4_macos64_llvm-5.0.tar.gz -C $THIRD_PARTY_DIR/paraview

  # Remove the downloaded tar files
  rm -rf *.tar.gz

  echo "Installation of prerequisites finished"
  echo "In every terminal you want to build or use BioDynamo execute:"
  echo "    source $BDM_ENVIRONMENT_FILE"
  echo ""
}

echo ""

# ask user if she really wants to perform this changes
# https://stackoverflow.com/questions/226703/how-do-i-prompt-for-yes-no-cancel-input-in-a-linux-shell-script
while true; do
  read -p "This script will install BioDynaMo in the directory /opt/biodynamo.
  Do you want to continue? (y/n) " yn
  case $yn in
    [Yy]* ) Install; exit;;
    [Nn]* ) exit;;
        * ) echo "Please answer yes or no.";;
  esac
done
