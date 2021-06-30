#!/bin/bash
# -----------------------------------------------------------------------------
#
# Copyright (C) 2021 CERN & Newcastle University for the benefit of the
# BioDynaMo collaboration. All Rights Reserved.
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

if [ "$(uname)" = "Darwin" ]; then
    export PV_SUPERBUILD_VERSION="v5.9.0"
    export PV_VERSION="v5.9.0"
    export QT_VERSION="v5.12.10"
else
    export PV_SUPERBUILD_VERSION="v5.9.0"
    export PV_VERSION="v5.9.0"
    export QT_VERSION="v5.12.10"
fi
export PV_GIT_REPO="https://gitlab.kitware.com/paraview/paraview.git"

export BDM_OS=$(DetectOs)
export QT_INSTALL_DIR="$WORKING_DIR/qt"

