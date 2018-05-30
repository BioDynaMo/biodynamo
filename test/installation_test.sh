#!/bin/bash

if [ $# -ne 1 ]; then
  echo "Wrong number of arguments.
Description:
  Run installation tests
Usage:
  installation_test.sh BDM_PROJECT_DIR
Arguments:
  BDM_PROJECT_DIR path to the biodynamo project directory
  "
  exit 1
fi

set -e -x

BDM_PROJECT_DIR=$1
cd $BDM_PROJECT_DIR

# speed-up build by disabling tests and demos
export BDM_CMAKE_FLAGS="-Dtest=off -Ddemo=off"

# Currently we are inside the biodynamo project directory, mapped as volume
# from the host
./install.sh . << EOF
y
EOF

# reload shell and source biodynamo
set +e +x
. $BDM_PROJECT_DIR/cmake/installation/common/util.sh
. $(BashrcFile)
$use_biodynamo
set -e -x

# verify if out of source builds work
cd ~
biodynamo new test-sim --no-github
cd test-sim
biodynamo run &>all

# extract ouput
cat all | tail -n3 | head -n1 >actual

# create file with expected output
echo "Simulation completed successfully!" >> expected

diff expected actual
exit $?
