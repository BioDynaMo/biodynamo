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

## Ubuntu 16.04, 18.04 and 20.04

### Required Packages

  * **wget**: Retrieves files from the web
  * **cmake**: Set of tools for automate building, testing of software
  * **make**: Build automation tool
  * **gcc**: GNU C compiler
  * **g++**: GNU C++ compiler
  * **libopenmpi-dev**: Development files for OpenMPI (Open Source Message Passing Interface)
  * **libomp-dev**: Development files for OpenMP (API for multiprocessor programming)
  * **libnuma-dev**: Development files for NUMA (simple programming interface to the policy supported by the Linux kernel)
  * **freeglut3-dev**: Development files for GLUT (OpenGL Utility Toolkit)
  * **libpthread-stubs0-dev**: Development files for managing threads

### Required package for custom Python installation with pyenv

  * **curl**: Command line tool for transferring data with URL syntax
  * **libssl-dev**: Secure Sockets Layer toolkit - development files
  * **zlib1g-dev**: Compression library - development
  * **libbz2-dev**: High-quality block-sorting file compressor library - development
  * **libreadline-dev**: GNU readline and history libraries, development files
  * **libsqlite3-dev**: SQLite 3 development files
  * **xz-utils**: XZ-format compression utilities
  * **tk-dev**: Toolkit for Tcl and X11 (default version) - development files
  * **libffi-dev**: Foreign Function Interface library (development files)
  * **liblzma-dev**: XZ-format compression library - development files
  * **python-openssl**: Python wrapper around the OpenSSL library

### Optional Packages

  * **valgrind**: A suite of tools for debugging and profiling
  * **clang-format**: clang-based C++ style checker and formatter
  * **clang-tidy**: clang-based C++ “linter” tool
  * **doxygen**: Tool for generating documentation from annotated C++ sources
  * **graphviz**: Graph Visualization Software used optionally by Doxygen
  * **lcov**: Graphical front-end for GCC's coverage testing tool gcov
  * **gcovr**: Tool to test code coverage in programs
  * **libxml2-dev**: Development files for the GNOME XML library

### Installation

#### Required Packages

```bash
sudo apt-get update
sudo apt-get install -y wget cmake make gcc g++ \
libopenmpi-dev libomp-dev libnuma-dev freeglut3-dev \
libpthread-stubs0-dev
```

We install pyenv as the Python Version Management tool to be able to switch
to a supported Python environment for BioDynaMo. This will not interfere with
your system's Python installation.

```bash
sudo apt-get install -y curl libssl-dev zlib1g-dev libbz2-dev libreadline-dev \
  libsqlite3-dev xz-utils tk-dev libffi-dev liblzma-dev python-openssl

curl https://pyenv.run | bash
export PATH="$HOME/.pyenv/bin:$PATH"
eval "$(pyenv init -)"
PYTHON_CONFIGURE_OPTS="--enable-shared" pyenv install 3.6.9
```

#### Optional Packages

```bash
pyenv shell 3.6.9
pip install --user nbformat jupyter metakernel
sudo apt-get install -y valgrind \
  clang-format clang-tidy \
  doxygen graphviz lcov gcovr libxml2-dev
```

## CentOS 7

### Required Packages

 * **epel-release**: Provides a set of additional packages for Enterprise Linux
 * **ius-release**: Provides RPM packages for newer software versions for for Enterprise Linux distributions
 * **wget**: Retrieves files from the web
 * **cmake3**: Set of tools for automate building, testing of software
 * **libXt-devel**: Basic library for developing X11
 * **libXext-devel**: Library which contains a handful of X11 extensions
 * **devtoolset-7-gcc**: Compiler suite for C and C++
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

 * **lcov**: Graphical front-end for GCC's coverage testing tool gcov
 * **gcov**: Tool to test code coverage in programs
 * **llvm-toolset-7**: software collection that provides software from the LLVM suite
 * **llvm-toolset-7-clang-tools-extra**: software collection that provides `clang-format` and `clang-tidy`
 * **valgrind**: A suite of tools for debugging and profiling
 * **doxygen**: Tool for generating documentation from annotated C++ sources
 * **graphviz**: Graph Visualization Software used optionally by Doxygen
 * **llvm-toolset-6.0-llvm-devel llvm-toolset-6.0-llvm-static**: Modular compiler and toolchain
 * **libxml2-devel**: Development files for the GNOME XML library

### Installation

#### Required Packages

```bash
sudo yum update -y
sudo yum -y install centos-release-scl epel-release
sudo yum -y install https://centos7.iuscommunity.org/ius-release.rpm
sudo yum -y install wget cmake3 libXt-devel libXext-devel \
  devtoolset-7-gcc* numactl-devel \
  openmpi3-devel freeglut-devel git
```

We install pyenv as the Python Version Management tool to be able to switch
to a supported Python environment for BioDynaMo. This will not interfere with
your system's Python installation.

```bash
sudo yum install -y @development zlib-devel bzip2 bzip2-devel readline-devel sqlite \
  sqlite-devel openssl-devel xz xz-devel libffi-devel findutils

curl https://pyenv.run | bash
export PATH="$HOME/.pyenv/bin:$PATH"
eval "$(pyenv init -)"
PYTHON_CONFIGURE_OPTS="--enable-shared" pyenv install 3.6.9
```

#### Optional Packages

```bash
pyenv shell 3.6.9
pip install --user nbformat jupyter metakernel
sudo yum -y install lcov gcovr llvm-toolset-7 \
   llvm-toolset-7-clang-tools-extra doxygen graphviz valgrind freeglut-devel
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
sudo yum install -y llvm-toolset-6.0-llvm-devel llvm-toolset-6.0-llvm-static
sudo yum install -y libxml2-devel
```

## MacOS

Currently we only support macOS installations of BioDynaMo using Homebrew
(if you are using another package manager you will need to install the
equivalent packages).
But first, before doing anything else, install Xcode (from the App Store) and the command line tools,
using the command "xcode-select --install".

### Required Packages

 * **llvm**: LLVM compiler suite with also OpenMP compliant clang and clang++
 * **wget**: Retrieves files from the web
 * **cmake**: Set of tools for automate building, testing of software
 * **libomp**: Development files for OpenMP (API for multiprocessor programming)
 * **open-mpi**: Development files for OpenMP (API for multiprocessor programming)
 * **python**: Python 3 Interpreter

### Optional Packages

 * **doxygen**: Tool for generating documentation from annotated C++ sources
 * **graphviz**: Graph Visualization Software used optionally by Doxygen
 * **lcov**: Graphical front-end for GCC's coverage testing tool gcov
 * **gcovr**: Tool to test code coverage in programs

### Installation

#### Required Packages

```bash
brew update; brew upgrade
brew install libomp open-mpi git pyenv llvm wget cmake python || true

# Install Python 3.6.9 environment
eval "$(pyenv init -)"
PYTHON_CONFIGURE_OPTS="--enable-shared" pyenv install 3.6.9
```

#### Optional Packages

```bash
pyenv shell 3.6.9
pip install --user nbformat jupyter metakernel
brew install doxygen graphviz lcov gcovr || true
```
