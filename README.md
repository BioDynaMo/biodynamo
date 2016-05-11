# BioDynaMo
Biological Dynamic Modeller - Based on Cortex 3D (Cx3D)

[![Build Status](https://travis-ci.org/breitwieser/cx3d-cpp.svg?branch=master)](https://travis-ci.org/breitwieser/cx3d-cpp)

##Introduction:

The brain is an extremely complex system, consisting of approximately 100 billion neurons
that are connected to one another. The way these neurons are structured allows for very
efficient and robust function. For example, human face recognition outperforms any currently
available machine algorithm. One way to better understand this complex structure is to
elucidate how it arises during development. The improvements in computing technology in
the last few years have made it possible to use large-scale computer simulations to
investigate such developmental processes. However, the appropriate software that can fully
exploit the potentials of the state-of-the-art hardware remains to be implemented.

A currently available software solution to simulate neural development is Cx3D,
<https://www.ini.uzh.ch/~amw/seco/cx3d/>. However, this software is Java-based, and not
ideal for high-performance computing (HPC). In order to adapt Cx3D to support HPC, a
software that has similar functionalities as Cx3D but is coded in C++ is needed.


## Build Instructions

Check out code from this repository:
```
git clone https://github.com/BioDynaMo/biodynamo.git
cd biodynamo
```

Create build directory
```
mkdir build && cd build
```

Build
```
cmake ..
make
```

This commands build a shared library for biodynamo together with a test executable. Furthermore, dependencies
(GMP, gtest, ...) are automatically downloaded and built as well. They are defined in CMake as ExternalProjects

Run tests by executing `./runBiodynamoTests` or `make test`. The former one is the
preferred way as it gives more output.

### CMake

If you do not want to build the test executable, run CMake with the test switch set to off:
```
cmake .. -Dtest=off
```
If you change the value of this switch, make sure you delete `CMakeCache.txt` beforehand.

### Make Targets
`make clean` will clean all targets, also the external projects

`make bdmclean` will only clean the `biodynamo` and `runBiodynamoTests` targets

`make doc` will generate the Doxygen documentation in directory `build/doc`. It contains a html and latex version.
You can view the html version by opening `build/doc/html/index.html` in your browser.

### Simulation Tests
Reference files for the simulation outcome are stored as Json in directory `test/resources/`
In rare cases it might be necessary to update these files:
```
./runBiodynamoTests updateSimStateReferenceFiles
```
The reference files are then overwritten with the simulation result of the current execution. No assertions
will be performed! Therefore, handle this option with care!

After each run, simulation test execution time are stored together with the latest_commit id in the corresponding csv file in `test/resources/`