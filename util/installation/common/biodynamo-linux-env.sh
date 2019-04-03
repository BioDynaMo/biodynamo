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

# This function has a zero exit code if the version is below the required
# version. This function can be used inside an if statement:
#    if $(versionLessThan "$(gcc -dumpversion)" "5.4.0"); then
#       echo "gcc version less than 5.4.0"
#    fi
# Arguments:
#   $1 actual version
#   $2 required version
function versionLessThan {
  local VERSION=$1
  local REQUIRED=$2
  [ "$(printf '%s\n' "$REQUIRED" "$VERSION" | sort -V | head -n1)" != "$REQUIRED" ]
}

# BioDynaMo
export BDM_INSTALL_DIR=$(realpath $(dirname "${BASH_SOURCE[0]}"))
#   required environment variables for out of source simulations
#   used by cmake to find BioDynaMoConfig.cmake
export CMAKE_PREFIX_PATH=${BDM_INSTALL_DIR}/biodynamo/cmake:$CMAKE_PREFIX_PATH
#   used inside BioDynaMoConfig.cmake to find UseBioDynaMo.cmake
export BDM_CMAKE_DIR=${BDM_INSTALL_DIR}/biodynamo/share/cmake
export BDM_SRC_DIR=${BDM_INSTALL_DIR}/biodynamo/include
export PATH=${BDM_INSTALL_DIR}/biodynamo/bin:$PATH
export LD_LIBRARY_PATH=${BDM_INSTALL_DIR}/biodynamo/lib:$LD_LIBRARY_PATH

# ROOT
. ${BDM_INSTALL_DIR}/third_party/root/bin/thisroot.sh
export ROOT_INCLUDE_PATH="${ROOT_INCLUDE_PATH:+${ROOT_INCLUDE_PATH}:}${BDM_INSTALL_DIR}/biodynamo/include"

# ParaView
export ParaView_DIR=${BDM_INSTALL_DIR}/third_party/paraview/lib/cmake/paraview-5.5
export ParaView_LIB_DIR=${BDM_INSTALL_DIR}/third_party/paraview/lib
export PYTHONPATH=${ParaView_LIB_DIR}/python2.7/site-packages
export PV_PLUGIN_PATH=${BDM_INSTALL_DIR}/biodynamo/lib/pv_plugin
export PATH=${BDM_INSTALL_DIR}/third_party/paraview/bin:${PATH}
export LD_LIBRARY_PATH=${ParaView_LIB_DIR}:${LD_LIBRARY_PATH}

# QT
export Qt5_DIR=${BDM_INSTALL_DIR}/third_party/qt/lib/cmake/Qt5
export QT_QPA_PLATFORM_PLUGIN_PATH=${BDM_INSTALL_DIR}/third_party/qt/plugins
export LD_LIBRARY_PATH=${BDM_INSTALL_DIR}/third_party/qt/lib:${LD_LIBRARY_PATH}

# user install mkdocs
export PATH=$PATH:~/.local/bin
export PYTHONPATH=~/.local/lib/python2.7/site-packages:$PYTHONPATH

# CMake
export PATH=${BDM_INSTALL_DIR}/third_party/cmake-3.6.3/bin:${PATH}

# Compiler
if command -v g++-5 &>/dev/null; then
  export CC=gcc-5
  export CXX=g++-5
else
  export CC=gcc
  export CXX=g++
fi

# OpenMP
export OMP_PROC_BIND=true

# CentOs specifics
if [ `lsb_release -si` == "CentOS" ]; then
  # python
  if versionLessThan "$(python3 --version)" 'Python 3.2.0'; then
    export PATH=/opt/rh/rh-python36/root/bin:/opt/rh/llvm-toolset-7/root/usr/bin/:$PATH
  fi
  # gcc g++
  export PATH=/opt/rh/devtoolset-7/root/usr/bin:$PATH
  export LD_LIBRARY_PATH=/opt/rh/devtoolset-7/root/usr/lib:/opt/rh/devtoolset-7/root/usr/lib/dyninst:/opt/rh/devtoolset-7/root/usr/lib64:/opt/rh/devtoolset-7/root/usr/lib64/dyninst:$LD_LIBRARY_PATH
fi

echo "You have successfully sourced BioDynaMo's environment."
