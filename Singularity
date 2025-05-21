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


  apt-get -y update
  apt-get -y install apt-utils
  apt-get -y install software-properties-common
  apt-get -y install python python3 git curl make gcc g++ wget wamerican wget wamerican libffi-dev libncurses5-dev zlib1g zlib1g-dev bzip2 aptitude libreadline-dev libssl-dev libsqlite3-dev \
    openmpi-bin libopenmpi-dev libxkbcommon-x11-dev bsdmainutils clang clang-format clang-tidy doxygen graphviz libxml2-dev llvm-7 llvm-7-dev llvm-7-runtime valgrind libgsl-dev freeglut3-dev \
    libbz2-dev libnuma-dev libomp5 libomp-dev libopenmpi-dev libpthread-stubs0-dev zlib1g-dev libbz2-dev libffi-dev liblzma-dev libreadline-dev libsqlite3-dev libssl-dev python-openssl tk-dev \
    xz-utils zlib1g-dev sudo libblas-dev liblapack-dev nano locales locales-all ninja-build python3-pip freeglut3-dev valgrind xvfb
  
  CMAKE_VER=3.19.3
  CMAKE_SH="cmake-${CMAKE_VER}-linux-x86_64.sh"
  curl -L -O  https://github.com/Kitware/CMake/releases/download/v${CMAKE_VER}/${CMAKE_SH}
  bash "${CMAKE_SH}" --prefix=/usr/local --skip-license
  rm "${CMAKE_SH}"


  dpkg-reconfigure locales
  locale-gen
  
  wget https://repo.continuum.io/miniconda/Miniconda3-latest-Linux-x86_64.sh
  bash Miniconda3-latest-Linux-x86_64.sh -b -f -p /miniconda3/
  rm Miniconda3-latest-Linux-x86_64.sh

  export PATH="/miniconda3/bin:$PATH"
  conda install -y -c conda-forge pip numpy 
  conda update -y --all

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

  # pip install cmake --upgrade
  pyenv global 3.9.1
  
  git config --system user.name "Test User"
  git config --system user.email user@test.com

  export BDM_BRANCH="master"
  git clone https://github.com/BioDynaMo/biodynamo.git
  cd biodynamo
  git checkout $BDM_BRANCH

  mkdir build
  cd build

  cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
  
  ninja -j $(($(nproc) - 1)) 
  

%runscript
