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

. /opt/biodynamo/third_party/root/bin/thisroot.sh
export CC=gcc-5
export CXX=g++-5
export ParaView_DIR=/opt/biodynamo/third_party/paraview/lib/cmake/paraview-5.4
export Qt5_DIR=/opt/biodynamo/third_party/qt/lib/cmake/Qt5
export LD_LIBRARY_PATH=/opt/biodynamo/third_party/qt/lib:/usr/lib/openmpi/lib:${LD_LIBRARY_PATH}
export PYTHONPATH=/opt/biodynamo/third_party/paraview/lib/paraview-5.4/site-packages:/opt/biodynamo/third_party/paraview/lib/paraview-5.4/site-packages/vtk
export QT_QPA_PLATFORM_PLUGIN_PATH=/opt/biodynamo/third_party/qt/plugins
export PATH=/opt/biodynamo/third_party/cmake-3.6.3/bin:/opt/biodynamo/third_party/paraview/bin:${PATH}

# required environment variables for out of source simulations
#   used by cmake to find BioDynaMoConfig.cmake
export CMAKE_PREFIX_PATH=/opt/biodynamo/cmake:$CMAKE_PREFIX_PATH
#   used inside BioDynaMoConfig.cmake to find UseBioDynaMo.cmake
export BDM_CMAKE_DIR=/opt/biodynamo/biodynamo/share/cmake
export BDM_SRC_DIR=/opt/biodynamo/biodynamo/include
export PATH=/opt/biodynamo/biodynamo/bin:$PATH
export LD_LIBRARY_PATH=/opt/biodynamo/biodynamo/lib:$LD_LIBRARY_PATH

# user install mkdocs
export PATH=$PATH:~/.local/bin
export PYTHONPATH=~/.local/lib/python2.7/site-packages:$PYTHONPATH
