#!/bin/bash
# -----------------------------------------------------------------------------
#
# Copyright (C) 2021 CERN & University of Surrey for the benefit of the
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

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
. "${SCRIPT_DIR}/env.sh"

set -e -x

if [ "$(uname)" = "Darwin" ]; then
  # copy paraview binary from .app bundle to bin
  __BDM_INSTALL_SOURCE=$SCRIPT_DIR/build/install
  cp -f $__BDM_INSTALL_SOURCE/Applications/paraview.app/Contents/MacOS/paraview $__BDM_INSTALL_SOURCE/bin
  # create install destination dir
  mkdir $SCRIPT_DIR/paraview
  # run the fix. Note the different calls on arm and i386 based machines.
  if [ "$(uname -p)" = 'arm' ]; then
    python3 $SCRIPT_DIR/make_macos_pvsdk_relocatable.py \
      --source $__BDM_INSTALL_SOURCE \
      --dest $SCRIPT_DIR/paraview \
      --third-party $SCRIPT_DIR qt \
      --pv 5.10 --py 3.9
  else
    python3 $SCRIPT_DIR/make_macos_pvsdk_relocatable.py \
      --source $__BDM_INSTALL_SOURCE \
      --dest $SCRIPT_DIR/paraview \
      --third-party $SCRIPT_DIR qt \
      --pv 5.9 --py 3.9
  fi
  cd $SCRIPT_DIR/paraview
  # Replace dummy .app binary with slightly more functional version from bin.
  # It still won't work, it just won't contain any refereces to the source dir.
  rm Applications/paraview.app/Contents/MacOS/paraview
  cp bin/paraview Applications/paraview.app/Contents/MacOS
  # Get rid of old paraview.conf in the dummy .app
  rm Applications/paraview.app/Contents/Resources/paraview.conf

  MACOS_VERS=$(sw_vers | sed -n 's/ProductVersion://p' | cut -d . -f 1-2 | sed -e 's/^[[:space:]]*//')
  MACOS_ARCH=$(arch)
  BDM_OS_VERS=${BDM_OS}-${MACOS_VERS}-${MACOS_ARCH}
  PV_TAR=paraview_${PV_VERSION}_${BDM_OS_VERS}_${PV_FLAVOR}.tar.gz

else
  cd "$SCRIPT_DIR/build/install"

  BDM_OS_VERS=${BDM_OS}
  PV_TAR=paraview_${PV_VERSION}_${BDM_OS_VERS}_${PV_FLAVOR}.tar.gz
fi

## tar the install directory
RESULT_FILE=paraview-$PV_VERSION-$BDM_OS-$PV_FLAVOR.tar.gz
tar -zcf $PV_TAR *
shasum -a256 ${PV_TAR} >${PV_TAR}.sha256

# After untarring the directory tree should like like this:
# paraview
#   |-- bin
#   |-- include
#   |-- lib
#   |-- share

# Step 5: cp to destination directory
cp ${PV_TAR} $BDM_PROJECT_DIR/build
cp ${PV_TAR}.sha256 $BDM_PROJECT_DIR/build
