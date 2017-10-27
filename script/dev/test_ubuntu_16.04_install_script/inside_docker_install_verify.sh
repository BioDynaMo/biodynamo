#!/bin/bash

BRANCH=$1

cd

git clone https://github.com/BioDynaMo/biodynamo
cd biodynamo/
git checkout $BRANCH

sudo script/install_prerequesites_ubuntu_16.04.sh

# source biodynamo environment
. /opt/biodynamo/biodynamo_dev.env

mkdir build
cd build
cmake .. && make check
sudo make install

# verify if out of source builds work
cd /tmp
git clone https://github.com/BioDynaMo/simulation-templates.git
cd simulation-templates
mkdir build && cd build
cmake ..
make -j4
./my-simulation >actual 2>&1

# create file with expected output
echo "Warning in <InitializeBioDynamo>: Config file bdm.toml not found." > expected
echo "Warning: No backup file name given. No backups will be made!" >> expected
echo "Your simulation objects are getting near the edge of the simulation space. Be aware of boundary conditions that may come into play!" >> expected
echo "Your simulation objects are getting near the edge of the simulation space. Be aware of boundary conditions that may come into play!" >> expected

diff expected actual
exit $?
