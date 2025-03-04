---
title: "Prerequisites"
date: "2019-01-01"
path: "/docs/userguide/prerequisites/"
meta_title: "BioDynaMo Prerequisites"
meta_description: "This is the prerequisites page."
toc: true
image: ""
next:
    url:  "/docs/userguide/prerequisites/"
    title: "Prerequisites"
    description: "This is the prerequisites page."
sidebar: "userguide"
keywords:
  -prerequisites
  -dependencies
  -third
  -party
  -packages
  -required
  -optional
---

This page lists the prerequisite packages that need to be installed in order to build correctly BioDynaMo.
BioDynaMo provides also an automated procedure to install all the needed libraries.

## Ubuntu 20.04 and 22.04

### Required Packages

  * **wget**: Retrieves files from the web
  * **curl**: Command line tool for transferring data with URL syntax
  * **cmake**: Set of tools for automate building, testing of software
  * **make**: Build automation tool
  * **gcc**: GNU C compiler
  * **g++**: GNU C++ compiler
  * **libblas-dev**: Development files for BLAS library (Basic Linear Algebra Subprograms)
  * **liblapack-dev**: Development files for LAPACK library (Linear Algebra PACKage)
  * **libopenmpi-dev**: Development files for OpenMPI (Open Source Message Passing Interface)
  * **libomp5**: OpenMP library
  * **libomp-dev**: Development files for OpenMP (API for multiprocessor programming)
  * **libnuma-dev**: Development files for NUMA (simple programming interface to the policy supported by the Linux kernel)
  * **freeglut3-dev**: Development files for GLUT (OpenGL Utility Toolkit)
  * **libpthread-stubs0-dev**: Development files for managing threads

### Required package for custom Python installation with pyenv

  * **libssl-dev**: Secure Sockets Layer toolkit - development files
  * **zlib1g-dev**: Compression library - development
  * **libbz2-dev**: High-quality block-sorting file compressor library - development
  * **libreadline-dev**: GNU readline and history libraries, development files
  * **libsqlite3-dev**: SQLite 3 development files
  * **xz-utils**: XZ-format compression utilities
  * **tk-dev**: Toolkit for Tcl and X11 (default version) - development files
  * **libffi-dev**: Foreign Function Interface library (development files)
  * **liblzma-dev**: XZ-format compression library - development files
  * **python(3)-openssl**: Python wrapper around the OpenSSL library

### Optional Packages

  * **valgrind**: A suite of tools for debugging and profiling
  * **clang-format**: clang-based C++ style checker and formatter
  * **clang-tidy**: clang-based C++ “linter” tool
  * **doxygen**: Tool for generating documentation from annotated C++ sources
  * **graphviz**: Graph Visualization Software used optionally by Doxygen
  * **kcov**: Code coverage testing tool (only on Ubuntu 20.04)
  * **libxml2-dev**: Development files for the GNOME XML library
  * **libgsl-dev**: GNU Scientific Library (GSL) -- development package  
  * **libgit2-dev**: C library for git 

### Installation

#### Required Packages

```bash
sudo apt-get update
sudo apt-get install -y wget curl make gcc g++ \
libblas-dev liblapack-dev libopenmpi-dev libomp5 libomp-dev \ 
libnuma-dev freeglut3-dev libpthread-stubs0-dev

curl -L -O https://github.com/Kitware/CMake/releases/download/v3.19.3/cmake-3.19.3-Linux-x86_64.sh
chmod +x cmake-3.19.3-Linux-x86_64.sh
sudo ./cmake-3.19.3-Linux-x86_64.sh --skip-license --prefix=/usr/local
```

We install pyenv as the Python Version Management tool to be able to switch
to a supported Python environment for BioDynaMo. This will not interfere with
your system's Python installation. On Ubuntu 20.04, run
```bash
sudo apt-get install -y libssl-dev zlib1g-dev libbz2-dev libreadline-dev \
  libsqlite3-dev xz-utils tk-dev libffi-dev liblzma-dev python-openssl
```
On Ubuntu 22.04, run
```bash
sudo apt-get install -y libssl-dev zlib1g-dev libbz2-dev libreadline-dev \
  libsqlite3-dev xz-utils tk-dev libffi-dev liblzma-dev python3-openssl
```
Afterwards, run the following command independent of the specific Ubuntu
version.
```bash
curl https://pyenv.run | bash
export PATH="$HOME/.pyenv/bin:$PATH"
eval "$(pyenv init --path)"
eval "$(pyenv init -)"
PYTHON_CONFIGURE_OPTS="--enable-shared" pyenv install 3.9.1
```

#### Optional Packages

```bash
pyenv shell 3.9.1
python -m pip install markupsafe==2.0.1 jupyter metakernel jupyterlab nbformat==5.4.0 nbconvert==6.5.3 nbclient==0.6.6
sudo apt-get install -y valgrind \
  clang-format clang-tidy \
  doxygen graphviz libxml2-dev libgsl-dev libgit2-dev
# on Ubuntu 20.04
sudo apt-get install -y kcov
```

## CentOS 7 (deprecated; no longer officially supported)

### Required Packages

 * **epel-release**: Provides a set of additional packages for Enterprise Linux
 * **ius-release**: Provides RPM packages for newer software versions for for Enterprise Linux distributions
 * **wget**: Retrieves files from the web
 * **cmake**: Set of tools for automate building, testing of software
 * **libXt-devel**: Basic library for developing X11
 * **libXext-devel**: Library which contains a handful of X11 extensions
 * **devtoolset-10-gcc**: Compiler suite for C and C++
 * **numactl-devel**: Development files for NUMA (simple programming interface to the policy supported by the Linux kernel)
 * **freeglut-devel**: Development files for GLUT (OpenGL Utility Toolkit)
 * **openmpi3-devel**: Development files for OpenMP (API for multiprocessor programming)

### Required package for custom Python installation with pyenv

 * **zlib-devel**: Header files and libraries for Zlib development
 * **bzip2**: A file compression utility
 * **bzip2-devel**: Libraries and header files for apps which will use bzip2
 * **readline-devel**: Files needed to develop programs which use the readline library
 * **sqlite**: Library that implements an embeddable SQL database engine
 * **sqlite-devel**: Development tools for the sqlite3 embeddable SQL database engine
 * **openssl-devel**: Files for development of applications which will use OpenSSL
 * **xz**: LZMA compression utilities
 * **xz-devel**: Devel libraries & headers for liblzma
 * **libffi-devel**: Development files for libffi
 * **findutils**: The GNU versions of find utilities (find and xargs)

### Optional Packages

 * **llvm-toolset-7**: software collection that provides software from the LLVM suite
 * **llvm-toolset-7-clang-tools-extra**: software collection that provides `clang-format` and `clang-tidy`
 * **llvm-toolset-7-llvm-devel**: LLVM developer libraries
 * **llvm-toolset-7-llvm-static**: LLVM static libraries
 * **valgrind**: A suite of tools for debugging and profiling
 * **doxygen**: Tool for generating documentation from annotated C++ sources
 * **graphviz**: Graph Visualization Software used optionally by Doxygen
 * **libxml2-devel**: Development files for the GNOME XML library
 * **gsl-devel**: GNU Scientific Library (GSL) -- development package
 * **atlas-devel**: Automatically Tuned Linear Algebra Software -- development package
 * **blas-devel**: The Basic Linear Algebra Subprograms library -- development package
 * **lapack-devel**: Numerical linear algebra package libraries -- development package

### Installation

#### Required Packages

```bash
sudo yum update -y
sudo yum -y install centos-release-scl epel-release
sudo yum -y install https://centos7.iuscommunity.org/ius-release.rpm
sudo yum -y install wget libXt-devel libXext-devel \
  devtoolset-10-gcc* numactl-devel \
  openmpi3-devel freeglut-devel git

curl -L -O https://github.com/Kitware/CMake/releases/download/v3.19.3/cmake-3.19.3-Linux-x86_64.sh
chmod +x cmake-3.19.3-Linux-x86_64.sh
./cmake-3.19.3-Linux-x86_64.sh --skip-license --prefix=/usr/local
```

We install pyenv as the Python Version Management tool to be able to switch
to a supported Python environment for BioDynaMo. This will not interfere with
your system's Python installation.

```bash
sudo yum install -y @development zlib-devel bzip2 bzip2-devel readline-devel \
  sqlite sqlite-devel openssl-devel xz xz-devel libffi-devel findutils

curl https://pyenv.run | bash
export PATH="$HOME/.pyenv/bin:$PATH"
eval "$(pyenv init --path)"
eval "$(pyenv init -)"
PYTHON_CONFIGURE_OPTS="--enable-shared" pyenv install 3.9.1
```

#### Optional Packages

```bash
pyenv shell 3.9.1
python -m pip install markupsafe==2.0.1 nbformat jupyter metakernel jupyterlab nbformat==5.4.0 nbconvert==6.5.3 nbclient==0.6.6
# SBML integration
sudo bash -c 'cat << EOF  > /etc/yum.repos.d/springdale-7-SCL.repo
[SCL-core]
name=Springdale SCL Base 7.6 - x86_64
mirrorlist=http://springdale.princeton.edu/data/springdale/SCL/7.6/x86_64/mirrorlist
#baseurl=http://springdale.princeton.edu/data/springdale/SCL/7.6/x86_64
gpgcheck=1
gpgkey=http://springdale.math.ias.edu/data/puias/7.6/x86_64/os/RPM-GPG-KEY-puias
EOF'
sudo yum update -y
sudo yum install -y doxygen graphviz valgrind freeglut-devel libxml2-devel
sudo yum install -y llvm-toolset-7 llvm-toolset-7-clang-tools-extra \
   llvm-toolset-7-llvm-devel llvm-toolset-7-llvm-static \
   gdl-devel atlas-devel blas-devel lapack-devel
```

## macOS

Requirements to build on macOS are:

 * A 64-bit Intel CPU or Apple Silicon CPU
 * macOS 12.0 or higher
 * [Xcode](https://itunes.apple.com/us/app/xcode/id497799835) and the Command Line Tools (CLT) for Xcode: `xcode-select --install`  
    (or from [developer.apple.com/downloads](https://developer.apple.com/downloads))
 * An up to date [Homebrew](https://brew.sh) installation (MacPorts and Fink are not supported)
 * A Bourne-compatible shell for installation (e.g. `bash` or `zsh`)

### Required Packages

 * **libomp**: Development files for OpenMP (API for multiprocessor programming)
 * **open-mpi**: Development files for OpenMP (API for multiprocessor programming)
 * **python@3.9**: Python interpreter
 * **wget**: Retrieves files from the web
 * **cmake**: Set of tools for automate building, testing of software
 * **ninja**: Ninja is a small build system with a focus on speed
 * **bash**: Recent version of bash shell
 * **tbb**: Rich and complete approach to parallelism in C++ (needed by ROOT on Apple M1)
 * **qt@5**: Library used by ParaView

### Optional Packages

 * **doxygen**: Tool for generating documentation from annotated C++ sources
 * **graphviz**: Graph Visualization Software used optionally by Doxygen
 * **kcov**: Code coverage testing tool
 * **gsl**: GNU Scientific Library (GSL) -- development package  
 * **libgit2**: C library for git 

### Installation

#### Required Packages

```bash
brew update; brew upgrade
brew install libomp open-mpi python@3.9 wget cmake ninja bash tbb qt@5
```

#### Optional Packages

```bash
brew install doxygen graphviz kcov gsl libgit2
python3 -m pip install markupsafe==2.0.1 nbformat jupyter metakernel jupyterlab jinja2==3.0
```
