#!/bin/bash
# -----------------------------------------------------------------------------
#
# Copyright (C) The BioDynaMo Project.
# All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
#
# See the LICENSE file distributed with this work for details.
# See the NOTICE file distributed with this work for additional information
# regarding copyright ownership.
#
# -----------------------------------------------------------------------------

if [[ $# -lt 2 ]]; then
  echo "Wrong number of arguments.
Description:
  Run a script inside a docker container
Usage:
  run-inside-docker.sh CONTAINER_OS SCRIPT [SCRIPT_ARGUMENTS]
Arguments:
  CONTAINER_OS OS id of the container
  SCRIPT absolute path to script that should be executed inside the container
         or relative path to BDM_PROJECT_DIR.
         NB: In both cases the script must be inside BDM_PROJECT_DIR
  SCRIPT_ARGUMENTS arguments that are passed to the script inside the docker
                   container (optional)
  "
  exit 1
fi

set -e

# save arguements in variables
BDM_OS=$1
shift
BDM_SCRIPT=$1
shift
BDM_SCRIPT_ARGUMENTS=$@

BDM_PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/.."

# include util functions
. $BDM_PROJECT_DIR/util/installation/common/util.sh

# check if BDM_OS is valid
CheckOsSupported $BDM_PROJECT_DIR/util/installation $BDM_OS

# check if BDM_OS supports docker
BDM_PATH_TO_DOCKERFILE=$BDM_PROJECT_DIR/util/installation/$BDM_OS
if [ ! -f "${BDM_PATH_TO_DOCKERFILE}/Dockerfile" ]; then
  echo "Could not find a Docker file in ${BDM_PATH_TO_DOCKERFILE}"
  echo "${BDM_OS} does not support Docker at the moment."
  exit 1
fi

BDM_CONTAINER=bdmdev-${BDM_OS}

# enables GUI apps
xhost +local:root &>/dev/null || true

EchoNewStep "Stop and remove any previously created $BDM_CONTAINER container..."
sudo docker stop $BDM_CONTAINER || true
sudo docker rm $BDM_CONTAINER || true

# create image from Dockerfile
echo ""
EchoNewStep "Build docker image..."
BDM_IMAGE=$BDM_CONTAINER
sudo docker build \
  --network=host \
  --build-arg HOST_UID=$(id -u `whoami`) \
  --build-arg HOST_GID=$(id -g `whoami`) \
  -t $BDM_IMAGE $BDM_PATH_TO_DOCKERFILE

echo ""
EchoNewStep "Start docker container..."
BDM_PROJECT_DIR_ABS=$(GetAbsolutePath $BDM_PROJECT_DIR)
# check if working directory is inside BDM_PROJECT_DIR
if [[ "$PWD" != $(realpath $BDM_PROJECT_DIR_ABS)* ]]; then
  EchoError "ERROR: working directory must be inside ${BDM_PROJECT_DIR_ABS}"
  echo "Current working directory: $PWD"
  echo "Change your working directory and run the script again."
  exit 1
fi

# BDM_LOCAL_LFS is defined add the environment variable and volume
if [ $BDM_LOCAL_LFS ]; then
  BDM_LOCAL_LFS_ENV="--env BDM_LOCAL_LFS=$BDM_LOCAL_LFS"
  BDM_LOCAL_LFS_VOLUME="--volume $BDM_LOCAL_LFS:$BDM_LOCAL_LFS"
fi
sudo docker run \
  --name $BDM_CONTAINER \
  --net=host \
  --env="DISPLAY" \
  $BDM_LOCAL_LFS_ENV \
  --volume $BDM_PROJECT_DIR_ABS:$BDM_PROJECT_DIR_ABS \
  --volume /var/run/docker.sock:/var/run/docker.sock \
  $BDM_LOCAL_LFS_VOLUME \
  --workdir $PWD \
  -dit \
  $BDM_IMAGE \
  /bin/bash

# execute script
# avoid exit if $BDM_SCRIPT returns non zero exit code;
# returning exit code is done manually afterwards
set +e
BDM_SCRIPT_ABS=$(GetAbsolutePath $BDM_SCRIPT)
echo ""
EchoNewStep "Execute ${BDM_SCRIPT}..."
sudo docker exec \
  -ti \
  $BDM_CONTAINER \
  $BDM_SCRIPT_ABS $BDM_SCRIPT_ARGUMENTS

RETURN_VAL=$?
echo ""
EchoNewStep "Finished"
echo "$BDM_SCRIPT return code was: $RETURN_VAL"
echo "The container '$BDM_CONTAINER' is still running."
echo "You can connect to it using 'sudo docker exec -ti $BDM_CONTAINER /bin/bash'"
exit $RETURN_VAL
