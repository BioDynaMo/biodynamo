# Installation

To install BioDynaMo for the first time execute the following commands.
The installation will also install all required packages including ParaView, ROOT and Qt5.

``` sh
git clone https://github.com/BioDynaMo/biodynamo.git
cd biodynamo
./install.sh
```

!!! important

    1. After the installation you need to restart your terminal.
      In every new terminal execute `source <path-to-bdm-installation>/biodynamo/bin/thisbdm.sh`
      to use BioDynaMo!

    2. BioDynaMo uses a customized version of ParaView.
	     Therefore, you should not install ParaView separately.

## Update Installation

The following commands update your BioDynaMo installation:

``` sh
cd path/to/biodynamo
# make sure you are on the master branch
git checkout master
# get latest changes
git pull origin master
./install.sh
```

## Supported platforms

*  **Ubuntu 16.04 (recommended)**, 18.04
*  CentOS 7.6.1810
*  Mac OSX

## Manual Installation

If you need more flexibility when installing BioDynaMo, it is also possible to build BioDynaMo manually. The complete
procedure is shown below:

```bash
git clone https://github.com/BioDynaMo/biodynamo.git
cd biodynamo

# Install the prerequisites for the project
./prerequisites.sh <your-os> all

# Create the build directory
mkdir build
cd build

# Build the project
cmake ../
make -j 4

# (Optional) Installs the library 
make install
```

The script `prerequisites.sh` is used to install all the dependencies needed by BioDynaMo. You will need
to run it before actually calling `cmake` and `make`. It will also choose the specific dependencies given the operative systems.
Run `./prerequisites.sh --help` to see how to use it.

Once the build is finished, it is possible to install the library in the given location (e.g the default will be `~/.bdm` ).
It is also possible to use the library without running `make install`. You will just need to source `thisbdm.sh` from the build
directory:

```bash

source build/biodynamo/bin/thisbdm.sh

```

## Advanced Build Options

#### Use a Custom Compiler
If you need to user a custom compilers (instead of the one automatically detected by BioDynaMo) you will need
to set first two variables: `CXX` for the C++ compiler and `CC` for the C compiler. Please not that your custom compiler
must support C++14 standard and also it must be compatible with OpenMP. The complete procedure will become:
```bash
git clone https://github.com/BioDynaMo/biodynamo.git
cd biodynamo

# Let's say I want to use a custom version of clang
export CXX=/opt/local/bin/clang++-mp-8.0
export C=/opt/local/bin/clang++-mp-8.0

./install.sh
```

#### Use a Custom ParaView/ROOT installation

BioDynaMo will download automatically the required ParaView, ROOT and Qt5 libraries to build the project. However,
it is also possible to specify custom version of them. You will need to set some environmental variables such to
enable this behaviour. Check out the example below.

```bash
git clone https://github.com/BioDynaMo/biodynamo.git
cd biodynamo

export ROOT_DIR=/opt/local/root
export ParaView_DIR=/opt/local/paraview
export Qt5_DIR=/usr/local/qt

./install.sh
```

!!! attention

    If you specify ParaView_DIR, then you will need to provide also the Qt5_DIR variable.
    This happens because ParaView needs to explicitly know where Qt is located.

