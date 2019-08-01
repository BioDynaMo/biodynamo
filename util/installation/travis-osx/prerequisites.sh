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
Arguments:
    <install_type>  all/required. If all is specified, then this script
                    will install all the prerequisites."
  exit 1
fi

# Install and upgrade required packages
brew install libomp tbb open-mpi git \
python python@2 llvm cmake || true

brew upgrade python cmake

# Install the optional packages
if [ $1 == "all" ]; then
    pip install --user mkdocs mkdocs-material
    brew install doxygen lcov gcovr || true
fi

# misc
# copy the omp.h file to our CMAKE_PREFIX_PATH
#OMP_V=`/usr/local/opt/llvm/bin/llvm-config --version`
#mkdir -p $BDM_INSTALL_DIR/biodynamo/include
#cp -f /usr/local/opt/llvm/lib/clang/$OMP_V/include/omp.h $BDM_INSTALL_DIR/biodynamo/include



