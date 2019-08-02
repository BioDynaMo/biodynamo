# Prerequisites

This page lists the prerequisites packages that needs to be installed in order to build correctly BioDynaMo.
BioDynaMo provides also an automated procedure to install all the needed library.

## Ubuntu 16.04, 18.04

### Required Packages

  * **cmake**: Set of tools for automate building, testing of software (for */usr/bin/cmake*);
  * **make**: Build automation tool (for */usr/bin/make*);
  * **gcc**: GNU C compiler (for */usr/bin/gcc*);
  * **g++**: GNU C++ compiler (for */usr/bin/g++*);
  * **libopenmpi-dev**: Development files for OpenMPI (Open Source Message Passing Interface);
  * **libomp-dev**: Development files for OpenMP (API for multiprocessor programming);
  * **libnuma-dev**: Development files for NUMA (simple programming interface to the policy supported by the Linux kernel);
  * **libtbb-dev**: Development files for TBB (C++ template library developed by Intel for parallel programming);
  * **libpthread-stubs0-dev**: Development files for managing threads;
  * **python3**: Python 3 Interpreter (for */usr/bin/python3*);
  * **python3-pip**: Python 3 Package Manager (for */usr/bin/pip3*).

### Optional Packages

  * **freeglut3-dev**: Development files for GLUT (OpenGL Utility Toolkit);
  * **valgrind**: A suite of tools for debugging and profiling (for */usr/bin/valgrind*);
  * **clang-format-3.9**: clang-based C++ style checker and formatter (for */usr/bin/clang-format-3.9*);
  * **clang-tidy-3.9**: clang-based C++ “linter” tool (for */usr/bin/clang-tidy-3.9*);
  * **doxygen**: Tool for generating documentation from annotated C++ sources (for */usr/bin/doxygen*);
  * **graphviz**: Graph Visualization Software used optionally by Doxygen;
  * **lcov**: Graphical front-end for GCC's coverage testing tool gcov;
  * **gcovr**: Tool to test code coverage in programs.

### Installation

#### Required Packages
```bash
sudo apt-get install -y cmake make gcc g++ \
libopenmpi-dev libomp-dev libnuma-dev libtbb-dev \
libpthread-stubs0-dev \
python3 python3-pip
```

#### Optional Packages
```bash
pip3 install --user mkdocs mkdocs-material
sudo apt-get install -y freeglut3-dev valgrind \
clang-3.9 clang-format-3.9 clang-tidy-3.9 \
doxygen graphviz \
lcov gcovr
```

## CentOS 7.6.1810

### Required Packages

 * **epel-release**: It provides a set of additional packages for Enterprise Linux;
 * **ius-release**: It provides RPM packages for newer software versions for for Enterprise Linux distributions;
 * **cmake3**: Set of tools for automate building, testing of software;
 * **libXt-devel**: Basic library for developing X11;
 * **libXext-devel**: Library which contains a handful of X11 extensions
 * **devtoolset-7-gcc**: Compiler suite for C and C++;
 * **numactl-devel**: Development files for NUMA (simple programming interface to the policy supported by the Linux kernel);
 * **tbb-devel**: Development files for TBB (C++ template library developed by Intel for parallel programming);
 * **openmpi3-devel**: Development files for OpenMP (API for multiprocessor programming);
 * **rh-python36**, **python** and **python-pip**: Python 3 Interpreter and Package Manager.


### Optional Packages

 * **freeglut-devel**: Development files for GLUT (OpenGL Utility Toolkit);
 * **lcov**: Graphical front-end for GCC's coverage testing tool gcov;
 * **gcov**: Tool to test code coverage in programs;
 * **llvm-toolset-7**: software collection that provides software from the LLVM suite;
 * **llvm-toolset-7-clang-tools-extra**: software collection that provides `clang-format` and `clang-tidy`;
 * **valgrind**: A suite of tools for debugging and profiling;
 * **doxygen**: Tool for generating documentation from annotated C++ sources;
 * **graphviz**: Graph Visualization Software used optionally by Doxygen.

### Installation

#### Required Packages
```bash
sudo yum update -y
sudo yum -y install centos-release-scl epel-release
sudo yum -y install https://centos7.iuscommunity.org/ius-release.rpm
sudo yum -y install cmake3 libXt-devel libXext-devel \
devtoolset-7-gcc* numactl-devel \
tbb-devel openmpi3-devel \
rh-python36 python python-pip
```
#### Optional Packages
```bash
pip install --user mkdocs mkdocs-material
sudo yum -y install lcov gcovr llvm-toolset-7\
llvm-toolset-7-clang-tools-extra doxygen graphviz valgrind freeglut-devel
```

## MacOS

!!! attention

    Currently we support only installation of BioDynaMo using Homebrew. If you are using
    another package manager you will need to install all the corresponding packages.

### Required Packages

 * **llvm**: LLVM compiler suite with also OpenMP compliant clang and clang++;
 * **cmake**: Set of tools for automate building, testing of software;
 * **libomp**: Development files for OpenMP (API for multiprocessor programming);
 * **tbb**: Development files for TBB (C++ template library developed by Intel for parallel programming);
 * **open-mpi**: Development files for OpenMP (API for multiprocessor programming);
 * **python** and **python@2**: Python 3 Interpreter.

### Optional Packages

 * **doxygen**: Tool for generating documentation from annotated C++ sources;
 * **lcov**: Graphical front-end for GCC's coverage testing tool gcov;
 * **gcovr**: Tool to test code coverage in programs.

### Installation

#### Required Packages

```bash
brew install libomp tbb open-mpi \
python python@2 llvm cmake || true

brew upgrade python cmake
```

#### Optional Packages

```bash
pip install --user mkdocs mkdocs-material
brew install doxygen lcov gcovr || true
```
