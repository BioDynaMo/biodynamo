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

# BioDynaMo
export BDM_INSTALL_DIR=@CMAKE_INSTALL_PREFIX@
#   required environment variables for out of source simulations
#   used by cmake to find BioDynaMoConfig.cmake
export CMAKE_PREFIX_PATH=${BDM_INSTALL_DIR}/biodynamo/cmake:$CMAKE_PREFIX_PATH
#   used inside BioDynaMoConfig.cmake to find UseBioDynaMo.cmake
export BDM_CMAKE_DIR=${BDM_INSTALL_DIR}/biodynamo/share/cmake
export BDM_SRC_DIR=${BDM_INSTALL_DIR}/biodynamo/include
export PATH=${BDM_INSTALL_DIR}/biodynamo/bin:$PATH
export DYLD_LIBRARY_PATH=${BDM_INSTALL_DIR}/biodynamo/lib:$DYLD_LIBRARY_PATH

# ROOT
. ${BDM_INSTALL_DIR}/third_party/root/bin/thisroot.sh

# Paraview
export ParaView_DIR=${BDM_INSTALL_DIR}/third_party/paraview/lib/cmake/paraview-5.5
export ParaView_LIB_DIR=${BDM_INSTALL_DIR}/third_party/paraview/lib
export PYTHONPATH=${ParaView_LIB_DIR}/python2.7/site-packages
export PV_PLUGIN_PATH=${BDM_INSTALL_DIR}/biodynamo/lib/pv_plugin
export PATH=${BDM_INSTALL_DIR}/third_party/paraview/bin:${PATH}
export DYLD_LIBRARY_PATH=${ParaView_LIB_DIR}:$DYLD_LIBRARY_PATH

# QT
export Qt5_DIR=${BDM_INSTALL_DIR}/third_party/qt/lib/cmake/Qt5
export QT_QPA_PLATFORM_PLUGIN_PATH=${BDM_INSTALL_DIR}/third_party/qt/plugins
export DYLD_LIBRARY_PATH=${BDM_INSTALL_DIR}/third_party/qt/lib:$DYLD_LIBRARY_PATH

# user install mkdocs
export PATH=$PATH:~/Library/Python/2.7/bin

# Compiler
export LLVMDIR="/usr/local/opt/llvm"
export CC=$LLVMDIR/bin/clang
export CXX=$LLVMDIR/bin/clang++
export CXXFLAGS=-I$LLVMDIR/include
export LDFLAGS=-L$LLVMDIR/lib
export PATH=$LLVMDIR/bin:$PATH

echo "You have successfully sourced BioDynaMo's environment."
