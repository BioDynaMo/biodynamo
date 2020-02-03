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

#####################################
## Building OpenMPI for BioDynaMo ##
#####################################

if [[ $# -ne 1 ]]; then
  echo "ERROR: Wrong number of arguments.
Description:
  This script builds OpenMPI.
  The archive will be stored in BDM_PROJECT_DIR/build/openmpi.tar.gz
Arguments:
  \$1 OpenMPI version that should be build (must be higher than 4.0.0)"
  exit 1
fi

set -e -x

OMPI_VERSION=$1
shift
BDM_PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../.."
cd $BDM_PROJECT_DIR

# archive destination dir
DEST_DIR=$BDM_PROJECT_DIR/build
mkdir -p $DEST_DIR
# working dir
WORKING_DIR=~/bdm-build-third-party
mkdir -p $WORKING_DIR
cd $WORKING_DIR

## Download OpenMPI official release source files
SRC_DIR=$WORKING_DIR/openmpi_src
mkdir -p $SRC_DIR
wget https://download.open-mpi.org/release/open-mpi/v4.0/openmpi-$OMPI_VERSION.tar.gz
tar -zxf openmpi-$OMPI_VERSION.tar.gz -C $SRC_DIR --strip-components=1

# The install directory
INSTALL_DIR=$WORKING_DIR/openmpi-$OMPI_VERSION
mkdir -p $INSTALL_DIR

cd $SRC_DIR
./configure \
  --prefix=$INSTALL_DIR \
  --enable-event-thread-support \
  --enable-opal-multi-threads \
  --enable-orte-progress-threads \
  --enable-mpi-thread-multiple \
  --enable-mpi-cxx

## Compile and install
make -j$(CPUCount) install

cd $INSTALL_DIR

## tar the install directory
tar -zcf openmpi-$OMPI_VERSION.tar.gz *

# cp to destination directory
cp openmpi-$OMPI_VERSION.tar.gz $DEST_DIR

echo "Instructions to distribute OpenMPI binaries:"
echo "To move an existing Open MPI installation to a new prefix: Set the OPAL_PREFIX "
echo "environment variable before launching Open MPI. For example, if Open MPI had "
echo "initially been installed to /opt/openmpi and the entire openmpi tree was later "
echo "moved to /home/openmpi, setting OPAL_PREFIX to /home/openmpi will enable Open MPI to function properly."
