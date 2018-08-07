A sample distributed simulation based on Ray Framework.

Quick start
===========

::

  mkdir build
  cd build
  cmake ..
  make -j $(nproc)
  python ../driver.py -l $PWD/libdistributed-ray.so -p 3-3-3
