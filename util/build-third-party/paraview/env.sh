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

# Parameters will be prepended here by util/build-third-party/build-paraview.sh

# import util functions
. "$BDM_PROJECT_DIR/util/installation/common/util.sh"

if [ "$PV_FLAVOR" = "default" ]; then
  export PV_SUPERBUILD_VERSION="v5.8.0"
  export PV_VERSION="v5.8.0"
  export PV_GIT_REPO="https://gitlab.kitware.com/paraview/paraview.git"
elif [ "$PV_FLAVOR" = "nvidia-headless" ]; then
  export PV_SUPERBUILD_VERSION="v5.8.0"
  export PV_VERSION="parallelize"
  export PV_GIT_REPO="https://github.com/breitwieserCern/paraview.git"
else 
  echo "ERROR: ParaView flavor ($PV_FLAVOR) is not supported"
  exit 1
fi

export BDM_OS=$(DetectOs)
export QT_INSTALL_DIR="$WORKING_DIR/qt"

