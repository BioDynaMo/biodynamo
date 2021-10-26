#!/bin/bash
# -----------------------------------------------------------------------------
#
# Copyright (C) 2021 CERN & University of Surrey for the benefit of the
# BioDynaMo collaboration. All Rights Reserved.
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
  This script builds roadrunner.
Arguments:
  \$1 Roadrunner version that should be build. Use a commit from branch llvm-7"
  exit 1
fi

set -e -x

# Base path
BDM_PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../.."

# archive destination dir
DEST_DIR=$BDM_PROJECT_DIR/build
mkdir -p $DEST_DIR
# working dir
WORKING_DIR=~/bdm-build-third-party
mkdir -p $WORKING_DIR
cd $WORKING_DIR

# Install the prerequisites
. ${BDM_PROJECT_DIR}/util/build-third-party/third-party-prerequisites.sh

EchoNewStep "Start building libroadrunner. Result will be stored in $DEST_DIR"

ROADRUNNER_BUILD_DIR=${BDM_PROJECT_DIR}/build/build-roadrunner
rm -rf $ROADRUNNER_BUILD_DIR
mkdir -p $ROADRUNNER_BUILD_DIR
cd $ROADRUNNER_BUILD_DIR

git clone https://github.com/sys-bio/roadrunner.git
git clone https://github.com/sys-bio/libroadrunner-deps

mkdir -p install/roadrunner

cd libroadrunner-deps/
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../../install/roadrunner ..
make -j`CPUCount` install

cd ../../roadrunner/

# Checkout to the branch compatible with llvm-7
git checkout $1

mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../../install/roadrunner \
  -DLLVM_CONFIG_EXECUTABLE=${LLVM_CONFIG} \
  -DTHIRD_PARTY_INSTALL_FOLDER=../../install/roadrunner \
  -DRR_USE_CXX11=OFF -DUSE_TR1_CXX_NS=ON \
  LIBSBML_LIBRARY=$ROADRUNNER_BUILD_DIR/install/roadrunner/lib/libsbml.so \
  LIBSBML_STATIC_LIBRARY=$ROADRUNNER_BUILD_DIR/roadrunner/install/roadrunner/lib/libsbml-static.a ..

make -j`CPUCount` install

# Create a tar file
GIT_COMMIT_HASH=`git rev-parse --short HEAD`
ROADRUNNER_TAR_FILE=libroadrunner-${GIT_COMMIT_HASH}.tar.gz
cd $ROADRUNNER_BUILD_DIR/install/roadrunner
tar -czf ${ROADRUNNER_TAR_FILE} *
mv ${ROADRUNNER_TAR_FILE} $DEST_DIR
EchoSuccess "The Libroadrunner tar file was successfully created at:"
EchoSuccess "$DEST_DIR/${ROADRUNNER_TAR_FILE}"
