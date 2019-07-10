#!/bin/bash
# -----------------------------------------------------------------------------
#
# Copyright (C) The BioDynaMo Project.
# All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
#
# See the LICENSE file distributed with this work for details.
# See the NOTICE file distributed with this work for additional information
# regarding copyright ownership.
#
# -----------------------------------------------------------------------------

if [[ $# -ne 1 ]]; then
  echo "ERROR: Wrong number of arguments.
Description:
    This script installs the prerequisites of BioDynaMo, but not BioDynaMo
    itself. Script install.sh installs both prerequisites and BioDynaMo.
No Arguments"
  exit 1
fi

# Install and upgrade required packages
brew install libomp tbb open-mpi git \
python python@2 llvm cmake || true

brew upgrade python cmake

# Install the optional packages
if [ $1 == "all" ]; then
    pip install --user mkdocs mkdocs-material
    brew install doxygen lcov gcov gcovr || true
fi

# misc
# copy the omp.h file to our CMAKE_PREFIX_PATH
#OMP_V=`/usr/local/opt/llvm/bin/llvm-config --version`
#mkdir -p $BDM_INSTALL_DIR/biodynamo/include
#cp -f /usr/local/opt/llvm/lib/clang/$OMP_V/include/omp.h $BDM_INSTALL_DIR/biodynamo/include

# Export path to make cmake find LLVM's clang (otherwise OpenMP won't work)
export LLVMDIR="/usr/local/opt/llvm"
export CC=$LLVMDIR/bin/clang
export CXX=$LLVMDIR/bin/clang++
export CXXFLAGS=-I$LLVMDIR/include
export LDFLAGS=-L$LLVMDIR/lib
export PATH=$LLVMDIR/bin:$PATH

# for mkdocs
export PATH=$PATH:~/Library/Python/2.7/bin

