#!/bin/bash
# This script installs the required packages on ubuntu 16.04 as outlined in the
# main README.md

function AddToBashrc {
  echo "Adding \"source ${BDM_ENVIRONMENT_FILE}\" to .bashrc"
  echo "source ${BDM_ENVIRONMENT_FILE}" >> ~/.bashrc
  . ~/.bashrc
}

function InstallCmake {
  wget https://cmake.org/files/v3.6/cmake-3.6.3-Linux-x86_64.tar.gz
  mkdir -p $THIRD_PARTY_DIR/cmake-3.6.3
  tar -xzf cmake-3.6.3-Linux-x86_64.tar.gz --strip 1 -C $THIRD_PARTY_DIR/cmake-3.6.3
  ln -s $THIRD_PARTY_DIR/cmake-3.6.3/bin/cmake /usr/bin/cmake
}

function Install {
  echo "Start installation of prerequisites..."

  THIRD_PARTY_DIR=$INSTALL_DIR/third_party

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

  apt-get update

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
  apt-add-repository -y "deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-3.9 main"
  apt-get update

  # install packages
  apt-get -y install libopenmpi-dev openmpi-bin
  apt-get -y install freeglut3-dev
  apt-get -y install git valgrind python python2.7-dev lcov
  apt-get -y install gcc-5 g++-5
  apt-get -y install clang-3.9 clang-format-3.9 clang-tidy-3.9 libomp-dev
  apt-get -y install doxygen graphviz

  # install ROOT
  wget -O /tmp/root_dict_patch.Linux-ubuntu16-x86_64-gcc5.4.tar.gz "https://cernbox.cern.ch/index.php/s/BbFptgxo2K565IS/download?path=%2F&files=root_dict_patch.Linux-ubuntu16-x86_64-gcc5.4.tar.gz"
  tar -xzf /tmp/root_dict_patch.Linux-ubuntu16-x86_64-gcc5.4.tar.gz -C $THIRD_PARTY_DIR

  # create environment script
  BDM_ENVIRONMENT_FILE=${THIRD_PARTY_DIR}/bdm_environment.sh
  touch ${BDM_ENVIRONMENT_FILE}

  echo ". ${THIRD_PARTY_DIR}/root/bin/thisroot.sh" >> ${BDM_ENVIRONMENT_FILE}

  # needed for Catalyst
  ln -s /usr/lib/libmpi.so /usr/local/lib/libmpi.so
  ln -s /usr/lib/libmpi.so /usr/local/lib/libmpi.so.12
  ln -s /usr/lib/openmpi/lib/libmpi.so /usr/lib/openmpi/lib/libmpi.so.1

  # install ParaView
  wget -O paraview-5.4_ubuntu14_gcc5.4_openmpi.tar.gz "https://cernbox.cern.ch/index.php/s/BbFptgxo2K565IS/download?path=%2F&files=paraview-5.4_ubuntu14_gcc5.4_openmpi.tar.gz"
  mkdir -p $THIRD_PARTY_DIR/paraview
  tar -xzf paraview-5.4_ubuntu14_gcc5.4_openmpi.tar.gz -C $THIRD_PARTY_DIR/paraview

  # install Qt
  wget -O Qt5.6.2_ubuntu16_gcc5.4.tar.gz "https://cernbox.cern.ch/index.php/s/BbFptgxo2K565IS/download?path=%2F&files=Qt5.6.2_ubuntu16_gcc5.4.tar.gz"
  mkdir -p $THIRD_PARTY_DIR/qt
  tar -xzf Qt5.6.2_ubuntu16_gcc5.4.tar.gz -C $THIRD_PARTY_DIR/qt

  echo 'export CC=gcc-5' >> ${BDM_ENVIRONMENT_FILE}
  echo 'export CXX=g++-5' >> ${BDM_ENVIRONMENT_FILE}

  # Set environmental variables for ParaView
  echo "export ParaView_DIR=$THIRD_PARTY_DIR/paraview/lib/cmake/paraview-5.4" >> ${BDM_ENVIRONMENT_FILE}
  echo "export Qt5_DIR=$THIRD_PARTY_DIR/qt/lib/cmake/Qt5" >> ${BDM_ENVIRONMENT_FILE}
  echo "export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$THIRD_PARTY_DIR/qt/lib" >> ${BDM_ENVIRONMENT_FILE}
  echo "export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/lib/openmpi/lib" >> ${BDM_ENVIRONMENT_FILE}
  echo "export PYTHONPATH=$THIRD_PARTY_DIR/paraview/lib/paraview-5.4/site-packages:$THIRD_PARTY_DIR/paraview/lib/paraview-5.4/site-packages/vtk" >> ${BDM_ENVIRONMENT_FILE}
  echo "QT_QPA_PLATFORM_PLUGIN_PATH=$THIRD_PARTY_DIR/qt/plugins" >> ${BDM_ENVIRONMENT_FILE}

  # Remove the downloaded tar files
  rm -rf *.tar.gz

  # add to ~/.bashrc
  if [ "$(cat ~/.bashrc | grep "source ${BDM_ENVIRONMENT_FILE}" | wc -l)" == "0" ]; then
    echo ""
    echo "+-------------------------------------------------------------------------+"
    echo "| Complete the installation by appending the following line to your       |"
    echo "| .bashrc file (e.g. gedit ~/.bashrc):                                    |"
    echo "|                                                                         |"
    echo "| source $BDM_DEPS            |"
    echo "|                                                                         |"
    echo "| And restart your terminal for the changes to take effect                |"
    echo "+-------------------------------------------------------------------------+"
    echo ""

    while true; do
      read -p "Or do you want to do this automatically? (y/n)" yn
      case $yn in
        [Yy]* ) AddToBashrc; exit;;
        [Nn]* ) exit;;
            * ) echo "Please answer yes or no.";;
      esac
    done
  fi
}

if [ "$(whoami)" != "root" ]; then
  echo "Error: This script requires root access. Exiting now."
  exit;
fi

# prompts user for installation directory
while true; do
  read -p "The default installation directory is /opt/biodynamo.
Do you want to change the installation directory? (y/n) " yn
  case $yn in
    [Yy]* ) INSTALL_DIR="$(zenity --file-selection --directory)"; break;;
    [Nn]* ) INSTALL_DIR=/opt/biodynamo; break;;
        * ) echo "Please answer yes or no.";;
  esac
done

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