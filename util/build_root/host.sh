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

# OPTIONS:
#  $1 image id: "travis" or "16.04"
#  $2 root commit_id

if [[ $# -ne 2 ]]; then
  echo "This script expects two arguments: "
  echo "OPTIONS: "
  echo "  $1 image id: \"travis\" or \"16.04\""
  echo "  $2 root commit_id"
  exit
elif [[ "$1" != "travis" ]] && [[ "$1" != "16.04" ]]; then
  echo "Image id must be either \"travis\" or \"16.04\""
  exit
fi

if [[ "$1" == "travis" ]]; then
  IMAGE=travisci/ci-garnet:packer-1496954857
elif [[ "$1" == "16.04" ]]; then
  IMAGE=ubuntu:16.04
fi

echo "Using docker image: ${IMAGE}"

# enables GUI apps
xhost +local:root

sudo docker stop build-root
sudo docker rm build-root

sudo docker run --name build-root --net=host --env="DISPLAY" -dit $IMAGE /sbin/init

# get path of this script
pushd `dirname $0` > /dev/null
SCRIPTPATH=`pwd`
popd > /dev/null

sudo docker cp $SCRIPTPATH/inside_docker.sh build-root:/
sudo docker exec -it build-root chmod +x /inside_docker.sh
sudo docker exec -it build-root ./inside_docker.sh $2

sudo docker cp build-root:/root/install/root.tar.gz ~/Downloads/root.tar.gz

echo "Build copied to ~/Downloads/root.tar.gz"
