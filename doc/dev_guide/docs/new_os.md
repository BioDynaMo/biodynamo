# Add support for a new Operating System

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
  * Add to `.travis.yml` two new tests for the newly added operating system. More specifically, you will need to
  add these two lines:
  ```yaml
  - os: linux
    before_install:
     - test -n $CC  && unset CC
     - test -n $CXX && unset CXX
    env: SCRIPT="util/travis-ci/installation-test.sh <your-os-name>-<version>"
  - os: linux
    compiler: gcc
    dist: trusty
    sudo: required
    group: edge
    env: SCRIPT="util/run-inside-docker.sh <your-os-name>-<version> util/travis-ci/default-build.sh"

  ```
