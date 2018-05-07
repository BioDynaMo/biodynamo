#!/bin/bash

# avoid permission denied
sudo chown -R "$USER":admin /usr/local

brew tap Biodynamo/biodynamo
brew install biodynamo

rm -rf simulation-templates
git clone https://github.com/BioDynaMo/simulation-templates.git
cd simulation-templates
mkdir build && cd build
source biodynamo.env
cmake ..
make -j4
./my-simulation >actual 2>&1

# create file with expected output
echo "Warning in <InitializeBiodynamo>: Config file bdm.toml not found in `.` or `../` directory." > expected
echo "Warning: No backup file name given. No backups will be made!" >> expected
echo "Your simulation objects are getting near the edge of the simulation space. Be aware of boundary conditions that may come into play!" >> expected
echo "Your simulation objects are getting near the edge of the simulation space. Be aware of boundary conditions that may come into play!" >> expected

diff expected actual
exit $?
