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

export PV_SUPERBUILD_VERSION="v5.8.1"
export PV_VERSION="v5.8.1"
export PV_GIT_REPO="https://gitlab.kitware.com/paraview/paraview.git"

export BDM_OS=$(DetectOs)
export QT_INSTALL_DIR="$WORKING_DIR/qt"

