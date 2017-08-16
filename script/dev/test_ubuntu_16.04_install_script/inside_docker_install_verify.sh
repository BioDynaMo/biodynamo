#!/bin/bash

BRANCH=$1

cd

git clone https://github.com/BioDynaMo/biodynamo
cd biodynamo/
git checkout $BRANCH

sudo script/install_prerequesites_ubuntu_16.04.sh

# manually source environment, due to issues with ~/.bashrc on docker
. /opt/biodynamo/third_party/bdm_environment.sh

mkdir build
cd build
cmake .. && make check
