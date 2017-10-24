#!/bin/bash

sudo snap install --dangerous --classic build-snap/biodynamo_0.1.0_amd64.snap

ldd /snap/bin/biodynamo.cmake
ls -al /snap/biodynamo/current/usr/lib/x86_64-linux-gnu/

echo "ldd"
ldd /snap/biodynamo/current/usr/bin/cmake

echo "biodynamo.run ldd..."
biodynamo.run "ldd /snap/biodynamo/current/usr/bin/cmake"

rm -rf simulation-templates
git clone https://github.com/BioDynaMo/simulation-templates.git
cd simulation-templates
mkdir build && cd build
biodynamo.cmake ..
biodynamo.make -j4
biodynamo.run ./my-simulation >actual 2>&1

# debug segfault
echo "backtrace" > ~/backtrace
echo "quit" >> ~/backtrace
echo "" >> ~/backtrace

sudo apt-get install gdb
echo '#!/bin/bash' > segfault_wrapper.sh
echo 'ulimit -c unlimited' >> segfault_wrapper.sh
echo '"$@"' >> segfault_wrapper.sh
echo 'gdb -q $1 core -x ~/backtrace' >> segfault_wrapper.sh
echo '' >> segfault_wrapper.sh
chmod +x segfault_wrapper.sh

./segfault_wrapper.sh biodynamo.cmake ..

# create file with expected output
echo "Warning in <InitializeBioDynamo>: Config file bdm.toml not found." > expected
echo "Warning: No backup file name given. No backups will be made!" >> expected
echo "Your simulation objects are getting near the edge of the simulation space. Be aware of boundary conditions that may come into play!" >> expected
echo "Your simulation objects are getting near the edge of the simulation space. Be aware of boundary conditions that may come into play!" >> expected

diff expected actual
exit $?
