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

set -x
# get path of this script
pushd `dirname $0` > /dev/null
SCRIPTPATH=`pwd`
popd > /dev/null

export PATH=$PATH:$SCRIPTPATH/../../util/makefile-build

cd $SCRIPTPATH/../../demo/makefile_project
make clean

if [ "$TRAVIS_OS_NAME" = "osx" ]; then
  # following line creates the cache for bdm-config
  # without it the make call fails on Travis OSX
  bdm-config --cxxflag
fi

make
./my-simulation 2>/dev/null | grep '^Simulation completed successfully!$'
exit $?
