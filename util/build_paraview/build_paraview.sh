#!/bin/bash
#####################################
## Building ParaView for BioDynaMo ##
#####################################

# Extract the desired ParaView version
if [[ $# -ne 1 ]] ; then
    echo 'Error: invalid arguments provided. First argument is desired ParaView version.'
    echo 'Make sure that this version matches the tag in ParaView code repository.'
    exit 1
fi
PV_VERSION=$1
PLATFORM=<find-platform>   # e.g. macos64 or ubuntu64
COMPILER=<find-compiler>   # e.g. llvm-5.0 or gcc-5.4.1
NUM_CORES=<number-of-cores>

## Step 1: Install Qt

# Using the silent JavaScript installer
QT_DIR=<placeholder>

## Step 2: Clone paraview github repository
git clone https://gitlab.kitware.com/paraview/paraview.git
cd paraview
git submodule update --init --recursive
git checkout v$PV_VERSION
git pull
git submodule update

## Step 3: Generate the cmake files for paraview
#
# If you want to build against a specific Qt library at /path/to/qt/cmake
# then set CMAKE_PREFIX_PATH=/path/to/qt/cmake (in this dir all the Qt modules
# should be available)

mkdir ../paraview-build
mkdir -p ../paraview-install/paraview
cd ../paraview-build

# The CMAKE_INSTALL_RPATH will put all the specified paths in all the installed
# targets (libraries and binaries) (upon make install). Since the relative paths
# from the ParaView targets are always the same we can set the rpaths to be
# relative from the ParaView targets (which are located at @loader_path). This
# makes the ParaView installation portable (as long as we copy Qt with it)
# DPARAVIEW_DO_UNIX_STYLE_INSTALLS forces CMake to install OSX build similarly
# to Linux, and enforces the RPATH (instead of @executable_path/../).
# The three RPATHS are respectively as follows:
# 1. ParaView binaries  -> Qt libraries
# 2. ParaView libraries -> Qt libraries
# 3. ParaView binaries / libraries -> ParaView libraries

Qt5_DIR=$QT_DIR cmake \
-DPARAVIEW_DO_UNIX_STYLE_INSTALLS:BOOL=ON \
-DCMAKE_MACOSX_RPATH:BOOL=ON \
-DCMAKE_INSTALL_PREFIX:STRING="../paraview-install" \
-DCMAKE_BUILD_TYPE:STRING="Release" \
-DPARAVIEW_ENABLE_PYTHON:BOOL=ON \
-DPARAVIEW_ENABLE_MPI:BOOL=OFF \
-DPARAVIEW_INSTALL_DEVELOPMENT_FILES:BOOL=ON \
-DCMAKE_INSTALL_RPATH:STRING="@loader_path/../../qt/lib;@loader_path/../../../../../qt/lib;@loader_path/../lib" \
 ../paraview

## Step 4: compile and install
make -j$NUM_CORES
make install -j$NUM_CORES

## Step 5: tar the install directory
cd ../paraview-install
tar -zcf paraview-$PV_VERSION_$PLATFORM_$COMPILER.tar.gz paraview

# After untarring the directory tree should like like this:
# paraview
#   |-- bin
#   |-- include
#   |-- lib
#   |-- share
