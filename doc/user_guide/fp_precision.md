---
title: "Floating-Point Precision"
date: "2022-07-18"
path: "/docs/userguide/fp-precision/"
meta_title: "BioDynaMo User Guide"
toc: true
sidebar: "userguide"
---

## Basics

BioDynaMo can be compiled with different floating point precisions.
Currently, single-precision (`float`) and double-precision (`double`) are supported.
By default, BioDynaMo is compiled with double-precision.

Reduced floating-point precision reduces the required main memory, the file size of simulation backups, and might reduce the simulation runtime.
 
You can print the used precision with:

```
bdm-config --fp-precision
```

or at the end of a simulation if you set `Param::statistics`to `true`.


To change the precision of BioDynaMo to `float`, add the cmake option `-Dreal_t=float` and (re)compile: 

```
cmake -Dreal_t=float ..
make -j<replace-with-num-cpus> 
```

## Usage in simulation code

If you want that the simulation adapts the floating-point precision to the BioDynaMo installation, use the following data types: 

* `real_t` (instead of `float` or `double`) and 
* `Real3` (instead of `Double3` or `Float3`).

If you switch the precision in BioDynaMo, you have to recompile the simulation (e.g., `bdm clean ; bdm build`)
