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

set -e -x

function test_local() {
    tmp_dir=$(mktemp -d)
    trap "rm -rf \"${tmp_dir}\"" EXIT

    biodynamo demo distributed "${tmp_dir}"
    cd "${tmp_dir}/distributed"
    mkdir build
    cd build
    cmake ..
    make -j $(nproc)
    python ../driver.py -l libdistributed-ray.so -m local
    python ../driver.py -l libdistributed-ray.so -m ray
}

function test_remote() {
    tmp_dir=$(mktemp -d)
    trap "sudo docker rm -f bdmdev-ray || true; rm -rf \"${tmp_dir}\"" EXIT

    biodynamo demo distributed "${tmp_dir}"
    cd "${tmp_dir}/distributed"
    mkdir build
    cd build
    cmake ..
    make -j $(nproc)
    cd ..
    sudo docker rm -f bdmdev-ray || true
    sudo docker build . \
        --network host \
        --build-arg USER="${USER}" \
        --build-arg HOST_UID="$(id -u `whoami`)" \
        --build-arg HOST_GID="$(id -g `whoami`)" \
        --tag bdmdev-ray
    port=$(($RANDOM % 50000 + 2000))
    sudo docker run \
        --name bdmdev-ray \
        --net host \
        --volume "${PWD}:${PWD}" \
        --volume "${BDM_INSTALL_DIR}:${BDM_INSTALL_DIR}" \
        --volume /tmp:/tmp \
        --env BDM_INSTALL_DIR="${BDM_INSTALL_DIR}" \
        --workdir "${PWD}" \
        -dit \
        bdmdev-ray \
        bash
    sudo docker exec \
        -it \
        bdmdev-ray \
        sudo pip install "${BDM_INSTALL_DIR}/third_party/ray/ray-0.5.0-cp27-cp27mu-linux_x86_64.whl"
    sudo docker exec \
        bdmdev-ray \
        bash -c "source ${BDM_INSTALL_DIR}/biodynamo-env.sh && ray start --head --redis-port ${port}" &
    # Wait for Ray to start
    sleep 5
    python driver.py -l "${PWD}/build/libdistributed-ray.so" --redis-address "127.0.0.1:${port}"
    sudo docker exec bdmdev-ray ray stop
}

test_local
if [ ! -f /.dockerenv ] ; then
  # Only run remote test if we are not already in a docker.
  test_remote
fi
