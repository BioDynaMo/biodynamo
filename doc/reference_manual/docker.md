# Docker

## Debugging with an Interactive Shell

```
# docker run -it <image> /bin/bash
# without --net=host internet is not working on a 15.04 host
sudo docker run -it --net=host ubuntu:16.04 /bin/bash

# execute the following commands inside the image
apt-get update
apt-get install -y g++-5 git
cd home
git clone https://github.com/BioDynaMo/biodynamo.git
cd biodynamo
script/install_prerequesites_ubuntu_16.04.sh
git checkout branch-name
mkdir build && cd build
cmake -DCMAKE_CXX_FLAGS="-O3" ..
make
make check
```
