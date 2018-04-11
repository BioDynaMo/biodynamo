#!/bin/bash
# This script installs the required packages on ubuntu 16.04 as outlined in the
# main README.md

INSTALL_DIR=/opt/biodynamo

function InstallCmake {
  wget https://cmake.org/files/v3.6/cmake-3.6.3-Linux-x86_64.tar.gz
  mkdir -p $THIRD_PARTY_DIR/cmake-3.6.3
  tar -xzf cmake-3.6.3-Linux-x86_64.tar.gz --strip 1 -C $THIRD_PARTY_DIR/cmake-3.6.3
  ln -s $THIRD_PARTY_DIR/cmake-3.6.3/bin/cmake /usr/bin/cmake
}

function Install {
  echo "Start installation of prerequisites..."

  THIRD_PARTY_DIR=$INSTALL_DIR/third_party

  # Remove everything in ${THIRD_PARTY_DIR} if it exists already.
  # Might contain outdated dependencies
  if [ -d "${THIRD_PARTY_DIR}" ]; then
    rm -rf "${THIRD_PARTY_DIR}/*"
  else
    mkdir -p $THIRD_PARTY_DIR
  fi

  # install `apt-add-repository and wget if not already installed
  # (missing on docker image)
  apt-get install -y software-properties-common wget

  # add repository for clang-3.9
  wget -O - http://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add -
  add-apt-repository -y ppa:ubuntu-toolchain-r/test  # gcc-5
  apt-add-repository -y "deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-3.9 main"
  apt-get update

  # If not installed, install CMake higher than the required version
  CMAKE_VERSION_R=3.3
  CMAKE_VERSION_I=`cmake --version | grep -m1 "" | sed -e 's/\<cmake version\>//g' -e "s/ //g"`
  if hash cmake 2>/dev/null; then
    rv=( ${CMAKE_VERSION_R//./ } )
    iv=( ${CMAKE_VERSION_I//./ } )
    if ! ((${iv[0]} >= ${rv[0]} && ${iv[1]} >= ${rv[0]})); then
      # disable the current install cmake by renaming it
      mv /usr/bin/cmake /usr/bin/cmake_$CMAKE_VERSION_I
      InstallCmake
    fi
  else
    InstallCmake
  fi

  # install packages
  apt-get -y install freeglut3-dev
  apt-get -y install git valgrind python python3 python2.7-dev lcov
  apt-get -y install gcc-5 g++-5 make
  apt-get -y install clang-3.9 clang-format-3.9 clang-tidy-3.9 libomp-dev
  apt-get -y install doxygen graphviz

  # install ROOT
  wget -O /tmp/root_v6.11.01_Linux-ubuntu16-x86_64-gcc5.4_263508429d.tar.gz "https://cernbox.cern.ch/index.php/s/1ekMudArMqFhcXt/download?path=%2F&files=root_v6.11.01_Linux-ubuntu16-x86_64-gcc5.4_263508429d.tar.gz"
  tar -xzf /tmp/root_v6.11.01_Linux-ubuntu16-x86_64-gcc5.4_263508429d.tar.gz -C $THIRD_PARTY_DIR

  # copy environment script
  #   get path of this script
  pushd `dirname $0` > /dev/null
  SCRIPTPATH=`pwd`
  popd > /dev/null
  BDM_ENVIRONMENT_FILE=/opt/biodynamo/biodynamo_dev.env
  cp $SCRIPTPATH/../cmake/biodynamo_linux_dev.env $BDM_ENVIRONMENT_FILE

  # install ParaView
  wget -O paraview-5.4_ubuntu14_gcc5.4.tar.gz "https://cernbox.cern.ch/index.php/s/1ekMudArMqFhcXt/download?path=%2F&files=paraview-5.4.1_ubuntu16_gcc5.4.tar.gz"
  mkdir -p $THIRD_PARTY_DIR/paraview
  tar -xzf paraview-5.4_ubuntu14_gcc5.4.tar.gz -C $THIRD_PARTY_DIR/paraview

  # install Qt
  wget -O Qt5.9.1_ubuntu16_gcc5.4.tar.gz "https://cernbox.cern.ch/index.php/s/1ekMudArMqFhcXt/download?path=%2F&files=Qt5.9.1_ubuntu16_gcc5.4.tar.gz"
  mkdir -p $THIRD_PARTY_DIR/qt
  tar -xzf Qt5.9.1_ubuntu16_gcc5.4.tar.gz -C $THIRD_PARTY_DIR/qt
  # temporal workaround to avoid libprotobuf error for paraview
  # use only until patched archive has been uploaded
  rm $THIRD_PARTY_DIR/qt/plugins/platformthemes/libqgtk3.so
  echo "" > $THIRD_PARTY_DIR/qt/lib/cmake/Qt5Gui/Qt5Gui_QGtk3ThemePlugin.cmake

  # Remove the downloaded tar files
  rm -rf *.tar.gz

  echo "Installation of prerequisites finished"
  echo "In every terminal you want to build or use BioDynamo execute:"
  echo "    source $BDM_ENVIRONMENT_FILE"
  echo ""
}

if [ "$(whoami)" != "root" ]; then
  echo "Error: This script requires root access. Exiting now."
  exit;
fi

echo ""

# ask user if she really wants to perform this changes
# https://stackoverflow.com/questions/226703/how-do-i-prompt-for-yes-no-cancel-input-in-a-linux-shell-script
while true; do
  read -p "This script adds the following package repository:
'deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-3.9 main'
and installs the packages mentioned in the main README.md.
Do you want to continue? (y/n) " yn
  case $yn in
    [Yy]* ) Install; exit;;
    [Nn]* ) exit;;
        * ) echo "Please answer yes or no.";;
  esac
done
