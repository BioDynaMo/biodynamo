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

# Base path
SCRIPTPATH="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../.."

# Install the prerequisites
. ${SCRIPTPATH}/util/build-third-party/third-party-prerequisites.sh

install_roadrunner()
{
  mkdir ${SCRIPTPATH}/build_roadrunner
  cd ${SCRIPTPATH}/build_roadrunner

  git clone https://github.com/sys-bio/roadrunner.git
  git clone https://github.com/sys-bio/libroadrunner-deps

  mkdir -p install/roadrunner

  cd libroadrunner-deps/
  mkdir build && cd build
  cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../../install/roadrunner ..
  make -j4 && make install

  cd ../../roadrunner/

  # Checkout to the branch compatible with llvm-6
  git checkout llvm-6

  mkdir build && cd build
  cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../../install/roadrunner \
    -DLLVM_CONFIG_EXECUTABLE=$1 \
    -DTHIRD_PARTY_INSTALL_FOLDER=../../install/roadrunner \
    -DRR_USE_CXX11=OFF -DUSE_TR1_CXX_NS=ON \
    LIBSBML_LIBRARY=${SCRIPTPATH}/build_roadrunner/install/roadrunner/lib/libsbml.so \
    LIBSBML_STATIC_LIBRARY=${SCRIPTPATH}/build_roadrunner/roadrunner/install/roadrunner/lib/libsbml-static.a ..

  make -j4 && make install
}

# Compile the library
install_roadrunner ${LLVM_CONFIG}

# Create a tar file
timestamp=$(date +%Y-%m-%d_%H-%M-%S)
tar -cvf ${SCRIPTPATH}/roadrunner-master-${timestamp}.tar.gz ${SCRIPTPATH}/build_roadrunner/install/roadrunner
