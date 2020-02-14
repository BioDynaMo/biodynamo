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

This page lists the prerequisites packages that needs to be installed in order to build correctly BioDynaMo.
BioDynaMo provides also an automated procedure to install all the needed library.

## Ubuntu 16.04, 18.04

### Required Packages

  * **wget**: Retrieves files from the web;
  * **cmake**: Set of tools for automate building, testing of software;
  * **make**: Build automation tool;
  * **gcc**: GNU C compiler;
  * **g++**: GNU C++ compiler;
  * **libopenmpi-dev**: Development files for OpenMPI (Open Source Message Passing Interface);
  * **libomp-dev**: Development files for OpenMP (API for multiprocessor programming);
  * **libnuma-dev**: Development files for NUMA (simple programming interface to the policy supported by the Linux kernel);
  * **libpthread-stubs0-dev**: Development files for managing threads;

### Optional Packages

  * **freeglut3-dev**: Development files for GLUT (OpenGL Utility Toolkit);
  * **valgrind**: A suite of tools for debugging and profiling;
  * **clang-format-3.9**: clang-based C++ style checker and formatter;
  * **clang-tidy-3.9**: clang-based C++ “linter” tool;
  * **doxygen**: Tool for generating documentation from annotated C++ sources;
  * **graphviz**: Graph Visualization Software used optionally by Doxygen;
  * **lcov**: Graphical front-end for GCC's coverage testing tool gcov;
  * **gcovr**: Tool to test code coverage in programs.
  * **llvm-6.0 llvm-6.0-dev llvm-6.0-runtime**: Modular compiler and toolchain
  * **libxml2-dev**: Development files for the GNOME XML library


### Installation

#### Required Packages
```bash
sudo apt-get install -y wget cmake make gcc g++ \
libopenmpi-dev libomp-dev libnuma-dev freeglut3-dev \
libpthread-stubs0-dev

# Install dependencies to install Python with PyEnv
sudo apt-get install -y libssl-dev zlib1g-dev libbz2-dev libreadline-dev \
  libsqlite3-dev xz-utils tk-dev libffi-dev liblzma-dev python-openssl

curl https://pyenv.run | bash
export PATH="$HOME/.pyenv/bin:$PATH"
eval "$(pyenv init -)"
env PYTHON_CONFIGURE_OPTS="--enable-shared" pyenv install 3.6.9
```

We install pyenv as the Python Version Management to be able to switch
to a supported Python environment for BioDynaMo. This will not interfere with
your system's Python installation.

#### Optional Packages
```bash
pip install --user nbformat jupyter metakernel
sudo apt-get install -y freeglut3-dev valgrind \
  clang-3.9 clang-format-3.9 clang-tidy-3.9 \
  doxygen graphviz lcov gcovr \
  llvm-6.0 llvm-6.0-dev llvm-6.0-runtime libxml2-dev
```

## CentOS 7

### Required Packages

 * **epel-release**: It provides a set of additional packages for Enterprise Linux;
 * **ius-release**: It provides RPM packages for newer software versions for for Enterprise Linux distributions;
 * **wget**: Retrieves files from the web;
 * **cmake3**: Set of tools for automate building, testing of software;
 * **libXt-devel**: Basic library for developing X11;
 * **libXext-devel**: Library which contains a handful of X11 extensions
 * **devtoolset-7-gcc**: Compiler suite for C and C++;
 * **numactl-devel**: Development files for NUMA (simple programming interface to the policy supported by the Linux kernel);
 * **openmpi3-devel**: Development files for OpenMP (API for multiprocessor programming);

We install pyenv as the Python Version Management to be able to switch
to a supported Python environment for BioDynaMo. This will not interfere with
your system's Python installation.

### Optional Packages

 * **freeglut-devel**: Development files for GLUT (OpenGL Utility Toolkit);
 * **lcov**: Graphical front-end for GCC's coverage testing tool gcov;
 * **gcov**: Tool to test code coverage in programs;
 * **llvm-toolset-7**: software collection that provides software from the LLVM suite;
 * **llvm-toolset-7-clang-tools-extra**: software collection that provides `clang-format` and `clang-tidy`;
 * **valgrind**: A suite of tools for debugging and profiling;
 * **doxygen**: Tool for generating documentation from annotated C++ sources;
 * **graphviz**: Graph Visualization Software used optionally by Doxygen.
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

# Install dependencies to install Python with PyEnv
sudo yum install -y @development zlib-devel bzip2 bzip2-devel readline-devel sqlite \
  sqlite-devel openssl-devel xz xz-devel libffi-devel findutils

curl https://pyenv.run | bash
export PATH="$HOME/.pyenv/bin:$PATH"
eval "$(pyenv init -)"
env PYTHON_CONFIGURE_OPTS="--enable-shared" pyenv install 3.6.9
```
#### Optional Packages
```bash
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

Currently we only test MacOS installations of BioDynaMo using Homebrew.
If you are using another package manager you will need to install all
the corresponding packages.

### Required Packages

 * **llvm**: LLVM compiler suite with also OpenMP compliant clang and clang++;
 * **wget**: Retrieves files from the web;
 * **cmake**: Set of tools for automate building, testing of software;
 * **libomp**: Development files for OpenMP (API for multiprocessor programming);
 * **tbb**: Development files for TBB (C++ template library developed by Intel for parallel programming);
 * **open-mpi**: Development files for OpenMP (API for multiprocessor programming);
 * **python** and **python@2**: Python 3 Interpreter.

### Optional Packages

 * **doxygen**: Tool for generating documentation from annotated C++ sources;
 * **graphviz**: Graph Visualization Software used optionally by Doxygen;
 * **lcov**: Graphical front-end for GCC's coverage testing tool gcov;
 * **gcovr**: Tool to test code coverage in programs.

### Installation

#### Required Packages

Using Homebrew:

```bash
brew install libomp tbb open-mpi git pyenv llvm wget cmake || true
brew upgrade python cmake || true

# Install Python 3.6.9 environment
eval "$(pyenv init -)"
env PYTHON_CONFIGURE_OPTS="--enable-shared" pyenv install 3.6.9
```

#### Optional Packages

Using Homebrew:

```bash
pip install --user nbformat jupyter metakernel
brew install doxygen graphviz lcov gcovr || true
```
