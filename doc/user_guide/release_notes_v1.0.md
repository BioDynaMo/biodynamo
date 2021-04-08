---
title: "BioDynaMo Version 1.0 Release Notes"
date: "2020-04-09"
path: "/docs/userguide/release_notes_v1.0/"
toc: true
image: ""
sidebar: "userguide"
keywords:
  -release
  -v1.0
  -1.0
---

BioDynaMo version 1.0 was released on April 9, 2021.

The following people have contributed to this first major version
(ordered by number of contributions):

* Lukas Breitwieser
* Ahmad Hesam
* Fons Rademakers
* Konstantinos Kanellis
* Jean de Montigny
* Roman Bauer
* Giovanni De Toni
* Jack Jennings
* Dorukhan Arslan
* Nam Nguyen
* Johannes Franz
* Lukasz Stempniewicz
* Robert Harakaly
* Sadyksaj
* Tobias Duswald
* Victor Drobny

## Main features

* A general API to implement and customize agent-based models
* A fully parallelized, high-performance simulation engine
* Ability to offload computations to GPUs of all major vendors
* Large-scale model support. BioDynaMo can simulate billions of agents on a single server.
* Predefined building blocks for simulations in neuroscience, oncology, and epidemiology
* Support for spherical and cylindrical agent geometries
* Diffusion methods: Euler, Runge-Kutta
* Multi-scale simulation support
* Visualization in ParaView
* Parameter configuration: JSON, C++ interpreter script, command line, or in-code
* Automatic backup and restore functionality using ROOT
* BioDynaMo Notebooks for rapid web-based prototyping


For a more detailed description of BioDynaMo's features and capabilities, 
please have a look at the following publication: 

**BioDynaMo: a general platform for scalable agent-based simulation** </br>
*Lukas Breitwieser, Ahmad Hesam, Jean de Montigny, Vasileios Vavourakis, Alexandros Iosif, Jack Jennings, Marcus Kaiser, Marco Manca, Alberto Di Meglio, Zaid Al-Ars, Fons Rademakers, Onur Mutlu, Roman Bauer* </br>
bioRxiv 2020.06.08.139949; doi: https://doi.org/10.1101/2020.06.08.139949


## Demos

BioDynaMo v1.0 comes with the following demos:

* Cell division
* Diffusion
* Soma clustering
* Tumor concept
* Epidemiology
* Gene regulation
* Parameters
* Multiple simulations
* Makefile project

## Supported Platforms

*  Ubuntu 18.04, 20.04
*  CentOS 7
*  macOS 10.15 and macOS 11.1

