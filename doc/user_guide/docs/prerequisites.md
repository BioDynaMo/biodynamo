# Prerequisites

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
  * **libtbb-dev**: Development files for TBB (C++ template library developed by Intel for parallel programming);
  * **libpthread-stubs0-dev**: Development files for managing threads;
  * **python3**: Python 3 Interpreter;
  * **python3-pip**: Python 3 Package Manager.

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
   libopenmpi-dev libomp-dev libnuma-dev libtbb-dev \
   libpthread-stubs0-dev python3 python3-pip
```

#### Optional Packages
```bash
pip3 install --user mkdocs mkdocs-material
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
 * **llvm-toolset-6.0-llvm-devel llvm-toolset-6.0-llvm-static**: Modular compiler and toolchain
 * **libxml2-devel**: Development files for the GNOME XML library

### Installation

#### Required Packages
```bash
sudo yum update -y
sudo yum -y install centos-release-scl epel-release
sudo yum -y install https://centos7.iuscommunity.org/ius-release.rpm
sudo yum -y install wget cmake3 libXt-devel libXext-devel \
   devtoolset-7-gcc* numactl-devel tbb-devel openmpi3-devel \
   rh-python36 python python-pip
```
#### Optional Packages
```bash
pip install --user mkdocs mkdocs-material
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

!!! attention

    Currently we support only installation of BioDynaMo using Homebrew and Fink.
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

Using HomeBrew:

```bash
brew install llvm wget cmake libomp tbb open-mpi python python@2 || true

brew upgrade python cmake || true
```

Using Fink:

```bash
sudo fink install wget llvm-clang cmake libomp-dev libtbb4 openmpi \
   python3 pip-py37
```

#### Optional Packages

Using HomeBrew:

```bash
pip install --user mkdocs mkdocs-material
brew install doxygen graphviz lcov gcovr || true
```

Using Fink:

```bash
pip install --user mkdocs mkdocs-material
sudo fink install doxygen graphviz
```
