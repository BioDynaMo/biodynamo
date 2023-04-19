Bootstrap:docker
From:ubuntu:20.04

%environment

%post -c /bin/bash

  export DEBIAN_FRONTEND=noninteractive 
  export TZ=Europe/Berlin
  unset Qt5_DIR
  unset ParaView_DIR
  unset ROOT_INCLUDE_PATH
  unset CMAKE_PREFIX_PATH
  unset PYENV_ROOT
  unset BDMSYS
  unset CC
  unset CXX


  apt-get -y update &&
  apt-get -y install python &&
  apt-get -y install python3 &&
  apt-get -y install git &&
  apt-get -y install curl &&
  apt-get -y install make &&
  apt-get -y install gcc &&
  apt-get -y install g++ &&
  apt-get -y install wget &&
  apt-get -y install wamerican && 
  apt-get -y install libffi-dev &&
  apt-get -y install libncurses5-dev &&
  apt-get -y install zlib1g &&
  apt-get -y install zlib1g-dev &&
  apt-get -y install bzip2 &&
  apt-get -y install zlib1g-dev && 
  apt-get -y install aptitude &&
  aptitude -y install libreadline-dev &&
  apt-get -y install libssl-dev &&
  apt-get -y install libsqlite3-dev &&
  apt-get -y install apt-utils &&
  apt-get -y install openmpi-bin &&
  apt-get -y install libopenmpi-dev &&
  apt-get -y install libxkbcommon-x11-dev &&
  apt-get -y install bsdmainutils &&
  apt-get -y install clang &&
  apt-get -y install clang-format &&
  apt-get -y install clang-tidy &&
  apt-get -y install doxygen &&
  apt-get -y install graphviz &&
  apt-get -y install libxml2-dev &&
  apt-get -y install llvm-7 &&
  apt-get -y install llvm-7-dev &&
  apt-get -y install llvm-7-runtime &&
  apt-get -y install valgrind &&
  apt-get -y install libgsl-dev &&
  apt-get -y install freeglut3-dev &&
  apt-get -y install libbz2-dev &&
  apt-get -y install libnuma-dev &&
  apt-get -y install libomp5 &&
  apt-get -y install libomp-dev &&
  apt-get -y install libopenmpi-dev &&
  apt-get -y install libpthread-stubs0-dev &&
  apt-get -y install zlib1g-dev &&
  apt-get -y install libbz2-dev &&
  apt-get -y install libffi-dev &&
  apt-get -y install liblzma-dev &&
  apt-get -y install libreadline-dev &&
  apt-get -y install libsqlite3-dev &&
  apt-get -y install libssl-dev &&
  apt-get -y install python-openssl &&
  apt-get -y install tk-dev &&
  apt-get -y install xz-utils &&
  apt-get -y install zlib1g-dev &&
  apt-get -y install sudo &&
  apt-get -y install software-properties-common &&
  apt-get -y install libblas-dev &&
  apt-get -y install liblapack-dev &&
  apt-get -y install nano

  apt-get -y install locales locales-all
  dpkg-reconfigure locales
  locale-gen
  
  wget https://repo.continuum.io/miniconda/Miniconda3-latest-Linux-x86_64.sh
  bash Miniconda3-latest-Linux-x86_64.sh -b -f -p /miniconda3/
  rm Miniconda3-latest-Linux-x86_64.sh

  export PATH="/miniconda3/bin:$PATH"
  conda install -y -c conda-forge pip numpy 
  conda update -y --all
  apt -y install git
  apt -y install curl

  export PYENV_ROOT="/opt/.pyenv"
  export PATH="$PYENV_ROOT/bin:$PATH" 
  curl -L https://github.com/pyenv/pyenv-installer/raw/master/bin/pyenv-installer | bash
  eval "$(pyenv init --path)"
  eval "$(pyenv init -)"
  PYTHON_CONFIGURE_OPTS="--enable-shared" pyenv install 3.9.1
  echo 'export PATH="/opt/.pyenv/versions/3.9.1/bin/:$PATH"' >> $SINGULARITY_ENVIRONMENT
  export PATH="/opt/.pyenv/versions/3.9.1/bin/:$PATH"

  pyenv shell 3.9.1

  wget https://bootstrap.pypa.io/get-pip.py
  python3 get-pip.py
  python3 -m pip install -U pip
  python3 -m pip install --upgrade pip
  python3 -m pip install numpy
  apt-get install -y python3-pip
  apt-get install -y freeglut3-dev valgrind

  pip install cmake --upgrade
  pyenv global 3.9.1
  
  apt-get -y dist-upgrade   
  sudo apt-get -y install ninja-build

  git clone https://github.com/BioDynaMo/biodynamo.git
  cd biodynamo

  mkdir build
  cd build

  cmake -G Ninja -Dparaview=OFF -DCMAKE_BUILD_TYPE=Release ..
  
  ninja -j $(($(nproc) - 1)) 
  

%runscript
