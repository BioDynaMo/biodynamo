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
  This script installs/updates the currently checked out version of biodynamo
Arguments:
  \$1 OS id"
  exit 1
fi

set -e

BDM_OS=$1
# remove argument so when we source prerequisites it won't complain about
# wrong number of arguments
shift

BDM_PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../../.."

# include util functions
. $BDM_PROJECT_DIR/util/installation/common/util.sh

# Custom instruction for MacOS (just in case)
# Export path to make cmake find LLVM's clang (otherwise OpenMP won't work)
if [ $BDM_OS = "osx" ]; then
    if [ -z ${CXX} ] && [ -z ${CC} ] ; then
        if [ -x "/usr/local/opt/llvm/bin/clang++" ]; then
            export LLVMDIR="/usr/local/opt/llvm"
            export CC=$LLVMDIR/bin/clang
            export CXX=$LLVMDIR/bin/clang++
            export CXXFLAGS=-I$LLVMDIR/include
            export LDFLAGS=-L$LLVMDIR/lib
            export PATH=$LLVMDIR/bin:$PATH
        elif [ -x "/opt/local/bin/clang++-mp-8.0" ]; then
            export CC=/opt/local/bin/clang++-mp-8.0
            export CXX=/opt/local/bin/clang-mp-8.0
        elif [ -x "/sw/opt/llvm-5.0/bin/clang++" ]; then
            export CC=/sw/opt/llvm-5.0/bin/clang++
            export CXX=/sw/opt/llvm-5.0/bin/clang
        fi
    fi

    # for mkdocs
    export PATH=$PATH:~/Library/Python/2.7/bin
fi

# Custom instructions for CentOS
set +e
if [ $BDM_OS = "centos-7.6.1810" ]; then
  export MESA_GL_VERSION_OVERRIDE=3.3
  . scl_source enable rh-python36
  if [ -z ${CXX} ] && [ -z ${CC} ] ; then
    . scl_source enable devtoolset-7
  fi

  . /etc/profile.d/modules.sh
  module load mpi
fi
set -e

# perform a clean release build
BUILD_DIR=$BDM_PROJECT_DIR/build
CleanBuild $BUILD_DIR

# print final steps
EchoSuccess "Installation of BioDynaMo finished successfully!"
EchoFinishThisStep $HOME/.bdm
