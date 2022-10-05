---
title: "BioDynaMo Version 1.04 Release Notes"
date: "2022-10-05"
path: "/docs/userguide/release_notes_v1.04/"
toc: true
image: ""
sidebar: "userguide"
keywords:
  -release
  -v1.04
  -1.04
---

BioDynaMo version 1.04 was released on October 5, 2022.

The following people have contributed to this version
(ordered by the number of contributions):

* Lukas Breitwieser
* Tobias Duswald
* Nicolò Cogno
* Fons Rademakers
* Ahmad Hesam
* Jack Jennings
* Moritz Grabmann

## New Features and Improvements

 * Add check for PVSM output files to system tests [#280](https://github.com/BioDynaMo/biodynamo/pull/280)
 * Generalize continuum integration [#260](https://github.com/BioDynaMo/biodynamo/pull/260)
 * Improve scheduler information [#277](https://github.com/BioDynaMo/biodynamo/pull/277)
 * Update flocking demo with local fluctuations [#256](https://github.com/BioDynaMo/biodynamo/pull/256)
 * Added check for decay rate in diffusion grid [#271](https://github.com/BioDynaMo/biodynamo/pull/271)
 * Make floating-point precision adjustable [#253](https://github.com/BioDynaMo/biodynamo/pull/253)
 * Improve UniformGridEnvironment::Box::Iterator interface
 * Add switch to turn off automatic sim size detection in UniformGridEnvironment
 * Add missing ForEachNeighbor implementation for UniformGridEnvironment
 * Add BinarySearch.DuplicatesLarge test
 * Remove const specifier from Agent::RemoveFromSimulation
 * Improve the time series collection API [#266](https://github.com/BioDynaMo/biodynamo/pull/266)
 * AgentPointer return nullptr directly for corresponding AgentUid
 * Make InPlaceExecutionContext::remove_ protected
 * Make Agent::RemoveFromSimulation virtual
 * Remove all virtual functions from InlineVector
 * Introduce different AgentPointer modes [#264](https://github.com/BioDynaMo/biodynamo/pull/264)
 * Improve mechanism to reuse the index part of AgentUids [#263](https://github.com/BioDynaMo/biodynamo/pull/263)
 * Update paraview build on macOS i386 [#261](https://github.com/BioDynaMo/biodynamo/pull/261)
 * Don't run Singularity GHA on all branches
 * Avoid parsing of `bdm.json` with `bdm test`
 * Add macOS packages to user guide
 * Extend DiffusionGrid interface for full gradient information [#257](https://github.com/BioDynaMo/biodynamo/pull/257)
 * Link Singularity in User Guide
 * Add left-multiplication to MathArray [#255](https://github.com/BioDynaMo/biodynamo/pull/255)
 * Minor code quality improvements [#252](https://github.com/BioDynaMo/biodynamo/pull/252)
 * Create BioDynaMo singularity image [#244](https://github.com/BioDynaMo/biodynamo/pull/244)
 * Reenable gitpod image workflow but only for manual triggering
 * Add additional LamdaFunctor tests
 * Add safety check in function PlotNeighborMemoryHistogram
 * Improve documentation of Agent::RemoveBehavior
 * Add coverage reporting with SonarCloud [#247](https://github.com/BioDynaMo/biodynamo/pull/247)
 * Upgrade to ParaView 5.10 on macOS [#235](https://github.com/BioDynaMo/biodynamo/pull/235)
 * Update continuum models [#241](https://github.com/BioDynaMo/biodynamo/pull/241)
 * Add improvements suggested by Sonar Cloud [#246](https://github.com/BioDynaMo/biodynamo/pull/246)
 * Add quality checks with `Sonar Cloud` [#245](https://github.com/BioDynaMo/biodynamo/pull/245)
 * Add SWC export for neurons [#243](https://github.com/BioDynaMo/biodynamo/pull/243)
 * Update link to endpoint package (CentOS CI)
 * Add CLI wrapper for `bdm-config` [#238](https://github.com/BioDynaMo/biodynamo/pull/238)
 * Add ProgressBar to visualise progress in `Simulate()` [#237](https://github.com/BioDynaMo/biodynamo/pull/237)
 * Remove `benchmark` from default build target [#236](https://github.com/BioDynaMo/biodynamo/pull/236)
 * Add SHA's for macOS 12.1 and Ubuntu 22.04.
 * Extend functionality of `show_simulation_step` parameter [#234](https://github.com/BioDynaMo/biodynamo/pull/234)

## Bug Fixes

 * Fix issue with markupsafe which breaks nbconvert and thus the website  [#281](https://github.com/BioDynaMo/biodynamo/pull/281)
 * Fix Python libraries on macOS [#279](https://github.com/BioDynaMo/biodynamo/pull/279)
 * Fix warning in multi-simulation mode [#272](https://github.com/BioDynaMo/biodynamo/pull/272)
 * Update GHA for macOS and fix notebooks on Ubuntu [#274](https://github.com/BioDynaMo/biodynamo/pull/274)
 * Fix notebook related packages on CentOS [#275](https://github.com/BioDynaMo/biodynamo/pull/275)
 * Fix issues introduced due to the global edit from double to `real_t` in OpenCL code.
 * Fix to find python3.9 on macOS.
 * Added version check and threshold fix for ParaView in epidemiology demo
 * Attribute environment update time to the correct operation
 * Reduce size of UniformGridEnvironment::Box
 * Remove obsolete parameter from toml file
 * Fix NUMA bug in ResourceManager::RemoveAgents
 * Fix GHA copyright script for linux
 * Fix DiffuseWithOpenEdge [#248](https://github.com/BioDynaMo/biodynamo/pull/248)
 * Add fixes for code-smells [#249](https://github.com/BioDynaMo/biodynamo/pull/249)
 * Fix bug in function GenerateSimulationInfoJson
 * Add additional AgentPointer tests; fix bug in AgentPointer
 * Fix possible race condition in user-defined thread-safety mechanism [#262](https://github.com/BioDynaMo/biodynamo/pull/262)
 * Fix normalization of the diffusion gradient [#259](https://github.com/BioDynaMo/biodynamo/pull/259)
 * Fix static agents bug in mechanical_forces_op
 * Fix compiler warnings [#254](https://github.com/BioDynaMo/biodynamo/pull/254)
 * Fix date-format in userguide/Singularity.md

## Examples

### New Demos

 * Cell monolayer growth [#278](https://github.com/BioDynaMo/biodynamo/pull/278)
 * Flocking Demo [#240](https://github.com/BioDynaMo/biodynamo/pull/240)

## Supported Platforms

 * Ubuntu 18.04, 20.04
 * CentOS 7
 * MacOS 11.7 and 12.6 (Intel and ARM) 

