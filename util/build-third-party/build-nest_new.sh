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

if [[ $# -ne 1 ]]; then
  echo "ERROR: Wrong number of arguments.
Description:
  This script builds NEST.
  The archive will be stored in BDM_PROJECT_DIR/build/.tar.gz
Arguments:
  \$1 BDM_PROJECT_DIR" 1>&2;
  exit 2
fi

NEST_VERSION_TGZ=nest-simulator-3.8


PYV2=`echo $PYVERS | cut -d . -f 1-2`

BDM_PROJECT_DIR=$1


# import util functions
. $BDM_PROJECT_DIR/util/installation/common/util.sh

NEST_INSTALL_DIR=$BDM_PROJECT_DIR/third_party/nest

NEST_TGZ_FILE=$BDM_PROJECT_DIR/third_party/$NEST_VERSION_TGZ.tar.gz

build_NEST(){
BDM_OS=$(DetectOs2)
PYTH=""
FEATURES=""
brewpy=no
if [ $BDM_OS == "osx" ]; then
  brewpy=yes
  if [ $brewpy == "yes" ]; then
  	 	 PYTH="$(brew --prefix)/bin/python${PYV2}"
	else
  		 export PYENV_NEST=$(brew --prefix)/opt/.pyenv
  		 eval "$(pyenv init -)"
  		 pyenv shell $PYVERS
  		 PYTH="`pyenv which python`"
	fi
else
	PYTH="`pyenv which python`"
fi
DEST_DIR=$BDM_PROJECT_DIR/build/build-third-party/build_nest
rm -rf $DEST_DIR
rm -rf $NEST_INSTALL_DIR
mkdir -p $DEST_DIR
mkdir -p $NEST_INSTALL_DIR
if ! [ -e $NEST_TGZ_FILE ]; then
  curl -o $NEST_TGZ_FILE https://github.com/nest/nest-simulator/archive/refs/tags/v3.8.tar.gz
fi
tar -xvzf $NEST_TGZ_FILE -C $DEST_DIR --strip-components=1
cd $DEST_DIR
mkdir obj
cd obj
echo $PYTH
cmake -DCMAKE_INSTALL_PREFIX=$NEST_INSTALL_DIR ..
cmake --build . --parallel $(CPUCount) --target install

return
}
build_NEST
exit 0;

done
