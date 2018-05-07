#!/bin/bash

set -e -x

BRANCH=$1

git config --global user.name "Test User"
git config --global user.email user@test.com

cd

git clone https://github.com/BioDynaMo/biodynamo
cd biodynamo/
git checkout $BRANCH

sudo script/install_prerequesites_ubuntu_16.04.sh << EOF
y
EOF

# source biodynamo environment
. /opt/biodynamo/biodynamo_dev.env

mkdir build
cd build
cmake -Dtest=off -Ddemo=off .. && make -j2
sudo make install

# verify if out of source builds work
biodynamo new test-sim --no-github
cd test-sim
biodynamo run &>all

# extract ouput
cat all | tail -n4 | head -n2 >actual

# create file with expected output
echo "Info: Initializing BiodynaMo..." > expected
echo "Simulation completed successfully!" >> expected

diff expected actual
exit $?
