A sample distributed simulation based on Ray Framework.

Quick start
===========

::

  mkdir build
  cd build
  cmake ..
  make -j $(nproc)
  python ../driver.py -l $PWD/libdistributed-ray.so -p 3-3-3


Tweaks to the driver
====================

There are some tweaks to the driver process that might be useful to conduct
benchmark.

1. The `object_store_memory` value should be set to approximately 4e9.
2. If all tasks should be run on one process, the `num_cpus` value should be
   set to 2 (and not 1). One worker will call `main` to initialize the global
   resource manager, and wait for the completion of the simuation. The other
   worker(s) is/are free to work on tasks.
3. Overhead of restarting a worker is controlled by the `max_calls` parameter
   to `@ray.remote` decorator.
