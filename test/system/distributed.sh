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

function clean_up_docker() {
    set +e
    sudo docker rm -f bdmdev-ray-head bdmdev-ray-worker
    sudo docker network rm bdm
    set -e
}

function test_remote() {
    tmp_dir=$(mktemp -d)
    trap "clean_up_docker; rm -rf \"${tmp_dir}\"" EXIT

    biodynamo demo distributed "${tmp_dir}"
    cd "${tmp_dir}/distributed"
    mkdir build
    cd build
    cmake ..
    make -j $(nproc)
    cd ..
    clean_up_docker
    sudo docker build . \
        --build-arg USER="${USER}" \
        --build-arg HOST_UID="$(id -u `whoami`)" \
        --build-arg HOST_GID="$(id -g `whoami`)" \
        --tag bdmdev-ray
    # Create a bridged network for head node and workers.
    sudo docker network create --driver bridge --subnet 10.11.12.0/24 bdm
    # Start the head node.
    port=$(($RANDOM % 50000 + 2000))
    head_ip=10.11.12.10
    sudo docker run \
        --name bdmdev-ray-head \
        --net bdm \
        --ip "${head_ip}" \
        --volume "${PWD}:${PWD}" \
        --volume "${BDM_INSTALL_DIR}:${BDM_INSTALL_DIR}" \
        --env BDM_INSTALL_DIR="${BDM_INSTALL_DIR}" \
        --workdir "${PWD}" \
        -dit \
        bdmdev-ray \
        bash
    sudo docker exec \
        -it \
        bdmdev-ray-head \
        sudo pip install "${BDM_INSTALL_DIR}/third_party/ray/ray-0.5.0-cp27-cp27mu-linux_x86_64.whl"
    # Commit the changes to bdmdev-ray image so that workers can use
    # the same image as the head node.
    sudo docker commit bdmdev-ray-head bdmdev-ray
    # Continue starting the head node.
    sudo docker exec \
        bdmdev-ray-head \
        bash -c "source ${BDM_INSTALL_DIR}/biodynamo-env.sh && ray start --head --redis-port ${port} --num-cpus=1" &
    # Wait for the head node to start.
    sleep 5
    # Then start the worker.
    worker_ip=10.11.12.11
    sudo docker run \
        --name bdmdev-ray-worker \
        --net bdm \
        --ip "${worker_ip}" \
        --volume "${PWD}:${PWD}" \
        --volume "${BDM_INSTALL_DIR}:${BDM_INSTALL_DIR}" \
        --env BDM_INSTALL_DIR="${BDM_INSTALL_DIR}" \
        --workdir "${PWD}" \
        -dit \
        bdmdev-ray \
        bash
    sudo docker exec \
        bdmdev-ray-worker \
        bash -c "source ${BDM_INSTALL_DIR}/biodynamo-env.sh && ray start --redis-address ${head_ip}:${port} --num-cpus=1" &
    # Wait for the worker node to start.
    sleep 5
    # Now start the driver on a different node.
    sudo docker run \
        --name bdmdev-ray-driver \
        --net bdm \
        --volume "${PWD}:${PWD}" \
        --volume "${BDM_INSTALL_DIR}:${BDM_INSTALL_DIR}" \
        --env BDM_INSTALL_DIR="${BDM_INSTALL_DIR}" \
        --workdir "${PWD}" \
        --rm \
        -it \
        bdmdev-ray \
        bash -c "source ${BDM_INSTALL_DIR}/biodynamo-env.sh && ray start --redis-address ${head_ip}:${port} --num-cpus=1 && python \"${PWD}/driver.py\" -l \"${PWD}/build/libdistributed-ray.so\" --redis-address \"${head_ip}:${port}\" && ray stop && cat /tmp/raylogs/*"
    sudo docker exec bdmdev-ray-worker bash -c "ray stop && cat /tmp/raylogs/*"
    sudo docker exec bdmdev-ray-head bash -c "ray stop && cat /tmp/raylogs/*"
}

test_local
if [ ! -f /.dockerenv ] ; then
  # Only run remote test if we are not already in a docker.
  test_remote
fi
