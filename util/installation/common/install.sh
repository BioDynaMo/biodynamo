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

# Custom instructions for CentOS (deprecated)
set +e
if [ $BDM_OS = "centos-7" ]; then
  # Turn of NUMA for Github Actions CentOS runner, because we get "mbind
  # operation not permitted errors", due to docker security constraints
  if [ ! -z ${GITHUB_ACTIONS+x} ]; then
    BDM_CMAKE_FLAGS="$BDM_CMAKE_FLAGS -Dnuma=off"
  fi

  if [ -z ${CXX} ] && [ -z ${CC} ] ; then
    . scl_source enable devtoolset-10
  fi

  . /etc/profile.d/modules.sh
  module load mpi
fi

if [ "$BDM_OS" != "osx" ]; then
  export PYENV_ROOT="$HOME/.pyenv"
  export PATH="$PYENV_ROOT/bin:$PATH"
  eval "$(pyenv init --path)"
  eval "$(pyenv init -)"
  pyenv shell 3.9.1
fi
set -e

# Test overriding the OS detection for one OS
if [ "${BDM_OS}" = "ubuntu-18.04" ]; then
  export BDM_CMAKE_FLAGS="$BDM_CMAKE_FLAGS -DOS=${BDM_OS}"
fi

# perform a clean release build
BUILD_DIR=$BDM_PROJECT_DIR/build
CleanBuild $BUILD_DIR

# print final steps
echo
EchoSuccess "Installation of BioDynaMo finished successfully!"

BDM_VERSION=$(cat $BUILD_DIR/version/bdm_shortversion)
INSTALL_DIR=${HOME}/biodynamo-v${BDM_VERSION}

EchoFinishInstallation $INSTALL_DIR
echo
