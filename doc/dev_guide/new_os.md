---
title: "Add support for a new Operating System"
date: "2019-01-01"
path: "/docs/devguide/new_os/"
meta_title: "BioDynaMo Dev Guide"
meta_description: "Information about supported OSes."
toc: true
image: ""
sidebar: "devguide"
keywords:
  -os
  -operating
  -system
---

In order to add BioDynaMo's support for a new operating systems, please follow the
steps presented below:

  * Add a new directory called `<your-os-name>-<version>` inside `util/installation`. Generally,
  `<your-os-name>` must be all lowercase and must match the result of the command `lsb_release -is`.
  Moreover, also `<version>` must match the result of the command `lsb_release -sr`;
  * Create a file called `prerequisites.sh` and place it inside `util/installation/<your-os-name>-<version>`.
  This file will install all the prerequisites needed by the new OS. The script must take only one argument
  which specifies which prerequisites will be installed (all of them or just the optional ones). Please have a
  look to the already existing `prerequisites.sh` for more information;
  * Add a `Dockerfile` which will be used to instantiate a container with your operating system for testing purposes.
  The `Dockerfile` needs also to be placed inside `util/installation/<your-os-name>-<version>`.
  * Add to `.github/workflows/` a new yml file for the newly added operating system. You can use [Act](https://github.com/nektos/act)
  to locally test your yml file.
