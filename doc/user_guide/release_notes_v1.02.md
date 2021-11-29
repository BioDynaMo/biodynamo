---
title: "BioDynaMo Version 1.02 Release Notes"
date: "2021-11-29"
path: "/docs/userguide/release_notes_v1.02/"
toc: true
image: ""
sidebar: "userguide"
keywords:
  -release
  -v1.02
  -1.02
---

BioDynaMo version 1.02 was released on November 29, 2021.

The following people have contributed to this version
(ordered by the number of contributions):

* Lukas Breitwieser
* Tobias Duswald
* Fons Rademakers
* Ahmad Hesam
* Will Hemsley

## New Features

* Enabled one-line install script to install arbitrary BioDynaMo versions [#231](https://github.com/BioDynaMo/biodynamo/pull/231)
* Add Scheduler function to print operation information
* Added parallelized optimization manager [#205](https://github.com/BioDynaMo/biodynamo/pull/205)
* Added googletest support for simulations [#187](https://github.com/BioDynaMo/biodynamo/pull/187)
* Improved biodynamo cli [#215](https://github.com/BioDynaMo/biodynamo/pull/215)
* Added experimental version of the automated benchmarking suite [#202](https://github.com/BioDynaMo/biodynamo/pull/202)
* Added CopyExecutionContext
* Added support for different execution orders
* Introduced execution context interface [#189](https://github.com/BioDynaMo/biodynamo/pull/189)
* Reduced memory consumption of the BDM memory manager [#186](https://github.com/BioDynaMo/biodynamo/pull/186)
* Added SphericalAgent
* Added toroidal space boundary condition
* Added RandomizedRm to randomize the iteration over all agents
* Added support for hierarchical agent-based models
* Added analysis classes to simplify data collection and plotting [#177](https://github.com/BioDynaMo/biodynamo/pull/177)
* Improved random number generation
* Added `Scheduler::SimulateUntil(exit_condition)`
* Added class LambdaFunctor and function L2F to simplify functor creation [#175](https://github.com/BioDynaMo/biodynamo/pull/175)
* Added octree and kd-tree as alternative environments [#169](https://github.com/BioDynaMo/biodynamo/pull/169)

## Bug Fixes

* Fixed environment inconsistencies [#226](https://github.com/BioDynaMo/biodynamo/pull/226)
* Added simulation dependent diffusion time step [#198](https://github.com/BioDynaMo/biodynamo/pull/198)
* Fixed bug in diffusion grid initialization [#199](https://github.com/BioDynaMo/biodynamo/pull/199)
* Fixed MathArray::Norm and Normalize for zero vector [#194](https://github.com/BioDynaMo/biodynamo/pull/194)
* Do not call `Rm::EndOfIteration` in `ExecCtxt::SetupIterationAll`
* Fixed errors in the static agent detection mechanism [#191](https://github.com/BioDynaMo/biodynamo/pull/191)
* Fixed race condition in `DiffusionGrid::ChangeConcentrationBy`
* Changed BioDynaMo version from `vXX.YY-ZZ-gSHA` to `vXX.YY.ZZ-SHA` [#216](https://github.com/BioDynaMo/biodynamo/pull/216)

## Examples

### New Demos

* Binding cells
* Pyramidal cell growth

### New Notebooks

* ST01-model-initializer.ipynb
* ST02-user-defined-random-number-distribution.ipynb
* ST03-agent-reproduction-mortality.ipynb
* ST04-agent-reproduction-with-behaviors.ipynb
* ST05-agent-reproduction-advanced.ipynb
* ST06-environment-search.ipynb
* ST07-multi-scale-simulation.ipynb
* ST08-histograms.ipynb
* ST09-timeseries-plotting-basic.ipynb
* ST10-timeseries-plotting-and-analysis.ipynb
* ST11-multiple-experiments-statistical-analysis.ipynb
* ST12-hierarchical-model.ipynb
* ST13-dynamic-scheduling.ipynb
* ST14-randomize-iteration-order.ipynb
* ST15-replace-interaction-force.ipynb

## Supported Platforms

*  Ubuntu 18.04, 20.04
*  CentOS 7
*  macOS 10.15, >=11.6 and >=12.0
