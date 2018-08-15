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

if [[ $# -ne 0 ]]; then
  echo "ERROR: Wrong number of arguments.
Description:
  This script creates ray.tar.gz file.
  The archive will be stored in BDM_PROJECT_DIR/build/ray.tar.gz
No Arguments"
  exit 1
fi

set -e -x

BDM_PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd $BDM_PROJECT_DIR

PN=ray
PV=0.5.0
P="${PN}-${PV}"
SRC_URI="https://github.com/ray-project/ray/archive/${P}.zip"
WORKDIR="${HOME}/bdm-build-third-party"
DISTDIR="${WORKDIR}"
ROOT="${WORKDIR}/${PN}"
DEST_DIR="${BDM_PROJECT_DIR}/build"

function fetch() {
    mkdir -p "${DISTDIR}"
    wget -c "${SRC_URI}" -O "${DISTDIR}/$(basename "${SRC_URI}")"
}

function pkg_setup() {
    sudo apt-get install -y cmake pkg-config build-essential curl libtool unzip
    sudo apt-get install -y flex bison python python-dev
    pip install cython
}

function src_unpack() {
    unzip -d "${WORKDIR}" "${DISTDIR}/$(basename "${SRC_URI}")"
}

function src_prepare() {
    pushd "${WORKDIR}/ray-${P}"
    git apply "${BDM_PROJECT_DIR}/util/build-third-party/ray/0001-Support-pre-assigning-return-IDs-to-remote-funcs.patch"
    popd
}

function src_compile() {
    pushd "${WORKDIR}/ray-${P}/python"
    python setup.py bdist_wheel
    popd
}

function src_install() {
    mkdir -p "${ROOT}"
    pushd "${WORKDIR}/ray-${P}"
    cp python/dist/*.whl "${ROOT}"
    cp -r thirdparty/pkg/boost/{include,lib} "${ROOT}/"
    cp -r thirdparty/pkg/flatbuffers/{include,lib} "${ROOT}/"
    cp -r thirdparty/pkg/arrow/cpp/build/cpp-install/{include,lib} "${ROOT}/"
    # Ray include files are very disorganized.
    mkdir -p "${ROOT}"/include/{common,local_scheduler,plasma,ray}
    mkdir -p "${ROOT}"/include/common/{format,state,thirdparty/ae}
    mkdir -p "${ROOT}"/include/common/thirdparty/hiredis
    mkdir -p "${ROOT}"/include/ray/{common,gcs/format,raylet/format,object_manager,util}
    cp src/common/*.h "${ROOT}/include/common"
    cp src/common/thirdparty/ae/*.h "${ROOT}/include/common/thirdparty/ae"
    cp src/common/thirdparty/hiredis/*.h "${ROOT}/include/common/thirdparty/hiredis"
    cp src/common/thirdparty/*.h "${ROOT}/include/common/thirdparty"
    cp src/common/format/*.h "${ROOT}/include/common/format"
    cp src/common/state/*.h "${ROOT}/include/common/state"
    cp src/local_scheduler/*.h "${ROOT}/include/local_scheduler"
    cp src/ray/*.h "${ROOT}/include/ray"
    cp src/ray/common/*.h "${ROOT}/include/ray/common"
    cp src/ray/gcs/*.h "${ROOT}/include/ray/gcs"
    cp src/ray/gcs/format/*.h "${ROOT}/include/ray/gcs/format"
    cp src/ray/raylet/*.h "${ROOT}/include/ray/raylet"
    cp src/ray/raylet/format/*.h "${ROOT}/include/ray/raylet/format"
    cp src/ray/util/*.h "${ROOT}/include/ray/util"
    cp src/plasma/*.h "${ROOT}/include/plasma"
    # Now copy ray libs, also terribly named (libcommon?).
    cp build/src/common/libcommon.a "${ROOT}/lib"
    cp build/src/local_scheduler/liblocal_scheduler_client.a "${ROOT}/lib"
    cp build/src/ray/libray.a "${ROOT}/lib"
    popd
}

fetch
pkg_setup
src_unpack
src_prepare
src_compile
src_install
pushd "${ROOT}/.."
tar czf "${DEST_DIR}/${PN}.tar.gz" "${PN}"
popd
