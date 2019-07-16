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

if [ $# -ne 1 ]; then
  echo "Wrong number of arguments.
Description:
  Run installation tests
Usage:
  installation-test.sh
No Arguments
  "
  exit 1
fi

set -e -x

source $(dirname "$BASH_SOURCE[0]")/util.inc

cd $BDM_PROJECT_DIR
# Currently we are inside the biodynamo project directory, mapped as volume
# from the host

# import util functions
. $BDM_PROJECT_DIR/util/installation/common/util.sh

# speed-up build by disabling tests and demos
export BDM_CMAKE_FLAGS="-Dtest=off"
./prerequisites.sh $1 all << EOF
y
EOF

# Operation needed in order to have a fully functional CentOS install procedure
# * Workaround for Faulty OpenGL version detection with software renderer
# * Enable python 3.6
# * Enable gcc and g++ compilers
# * Load mpi module
# The set +e is needed because scl_source relies on exit signal to detect if the
# corresponding packages was enabled. However, if we set -e, then the script
# will just exit without any error message.
set +e
if [ $1 = "centos-7.6.1810" ]; then
  export MESA_GL_VERSION_OVERRIDE=3.3
  . scl_source enable rh-python36
  . scl_source enable devtoolset-7

  . /etc/profile.d/modules.sh
  module load mpi
fi
set -e


# Build BioDynaMo
mkdir build
cd build
cmake ..
make -j$(CPUCount)
make install
cd ..

# reload shell and source biodynamo
set +e +x
source ~/.bdm/biodynamo-env.sh
set -e -x

# run system test.
test/system-test.sh

# verify if out of source builds work
cd ~
biodynamo new test-sim
run_cmake_simulation test-sim
