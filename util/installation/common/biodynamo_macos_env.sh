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

export LLVMDIR="/usr/local/opt/llvm"
export CC=$LLVMDIR/bin/clang
export CXX=$LLVMDIR/bin/clang++
export CXXFLAGS=-I$LLVMDIR/include
export LDFLAGS=-L$LLVMDIR/lib
export PATH=$LLVMDIR/bin:$PATH

export ParaView_DIR=/opt/biodynamo/third_party/paraview/lib/cmake/paraview-5.4
export Qt5_DIR=/usr/local/opt/qt/lib/cmake/Qt5
export DYLD_LIBRARY_PATH=$LLVMDIR/lib:$DYLD_LIBRARY_PATH
export DYLD_LIBRARY_PATH=/opt/biodynamo/biodynamo/lib:$DYLD_LIBRARY_PATH
export DYLD_LIBRARY_PATH=/opt/biodynamo/third_party/qt/lib:$DYLD_LIBRARY_PATH
export DYLD_LIBRARY_PATH=/opt/biodynamo/third_party/paraview/lib/paraview-5.4:$DYLD_LIBRARY_PATH
export PYTHONPATH=/opt/biodynamo/third_party/paraview/lib/paraview-5.4/site-packages:/opt/biodynamo/third_party/paraview/lib/paraview-5.4/site-packages/vtk
export QT_QPA_PLATFORM_PLUGIN_PATH=/opt/biodynamo/third_party/qt/plugins
export PATH=/opt/biodynamo/third_party/paraview/bin:/opt/biodynamo/third_party/paraview/bin/paraview.app/Contents/MacOS:${PATH}

# required environment variables for out of source simulations
#   used by cmake to find BioDynaMoConfig.cmake
export CMAKE_PREFIX_PATH=/opt/biodynamo/cmake:$CMAKE_PREFIX_PATH
#   used inside BioDynaMoConfig.cmake to find UseBioDynaMo.cmake
export BDM_CMAKE_DIR=/opt/biodynamo/share/biodynamo/cmake
export BDM_SRC_DIR=/opt/biodynamo/include/biodynamo
export PATH=/opt/biodynamo/bin:$PATH

# user install mkdocs
export PATH=$PATH:~/Library/Python/2.7/bin
