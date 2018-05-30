#!/bin/bash
set -x
# get path of this script
pushd `dirname $0` > /dev/null
SCRIPTPATH=`pwd`
popd > /dev/null

export PATH=$PATH:$SCRIPTPATH/../../cmake/non-cmake-build

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
