# cx3d-cpp
A port of Cortex 3D (Cx3D) to C++ to support HPC

[![Build Status](https://travis-ci.org/breitwieser/cx3d-cpp.svg?branch=master)](https://travis-ci.org/breitwieser/cx3d-cpp)

##Introduction:

The brain is an extremely complex system, consisting of approximately 100 billion neurons that are connected to one another. The way these neurons are structured allows for very efficient and robust function. For example, human face recognition outperforms any currently available machine algorithm. One way to better understand this complex structure is to elucidate how it arises during development. The improvements in computing technology in the last few years have made it possible to use large-scale computer simulations to investigate such developmental processes. However, the appropriate software that can fully exploit the potentials of the state-of-the-art hardware remains to be implemented.
A currently available software solution to simulate neural development is Cx3D. <https://www.ini.uzh.ch/~amw/seco/cx3d/>. However, this software is Java-based, and not ideal for high-performance computing (HPC). In order to adapt Cx3D to support HPC, a software that has similar functionalities as Cx3D but is coded in C++ is needed.

### Installation

This project is developed within a linux environment. To build cx3d-cpp you need the following software packages
installed on your computer - the tested version number is in parenthesis.

* JDK       (openjdk 1.8.0.51)
* Maven     (3.0.5)
* CMake     (2.8.11)
* SWIG      (3.0.7)
* libGMP    (6.0.0-11)

The C++ build process is fully integrated into the maven build lifecycle. Before the Java classes are compiled, the 
native library is compiled and SWIG generates the needed wrapper code.

In order to obtain the code, build and run the tests just execute the following commands in your terminal:

```bash
git clone https://github.com/breitwieser/cx3d-cpp.git
cd cx3d-cpp
mvn clean test
```
