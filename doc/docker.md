# BioDynaMo & Docker

### Debugging with an Interactive Shell

```
# docker run -it <image> /bin/bash
# without --net=host internet is not working on a 15.04 host
docker run -it --net=host ubuntu:16.04 /bin/bash
# execute the following commands inside the image
apt-get update
apt-get install -y g++-4.8 cmake valgrind git
cd home
git clone https://github.com/BioDynaMo/biodynamo.git
cd biodynamo
git checkout backend-arch
mkdir build && cd build
cmake ..
make
make check
```
