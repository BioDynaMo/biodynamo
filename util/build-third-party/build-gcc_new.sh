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
  This script builds GCC.
  The archive will be stored in BDM_PROJECT_DIR/build/gcc.tar.gz
Arguments:
  \$1 BDM_PROJECT_DIR" 1>&2;
  exit 2
fi

GCC_VERSION_TXZ=gcc-11.5.0


BDM_PROJECT_DIR=$1


# import util functions
. $BDM_PROJECT_DIR/util/installation/common/util.sh

GCC_INSTALL_DIR=$BDM_PROJECT_DIR/third_party/gcc

GCC_TXZ_FILE=$BDM_PROJECT_DIR/third_party/$GCC_VERSION_TXZ.tar.xz

build_gcc(){

DEST_DIR=$BDM_PROJECT_DIR/build/build-third-party/build_gcc
rm -rf $DEST_DIR
rm -rf $GCC_INSTALL_DIR
mkdir -p $DEST_DIR
mkdir -p $GCC_INSTALL_DIR
tar -Jxvf $GCC_TXZ_FILE -C $DEST_DIR --strip-components=1
cd $DEST_DIR
./configure --disable-multilib --prefix="$GCC_INSTALL_DIR"
make -j$(CPUCount)
make install
return
}

SPECIFIC_BDM_OS=$(DetectOs)

DEFAULT_GCCBD_OPTION="Yes"

if echo "$SPECIFIC_OSES_SUPPORTED" | grep -Eiq "$SPECIFIC_BDM_OS" ;  then
	DEFAULT_GCCBD_OPTION="No"
fi

while true; do
	read -p "Do you want to build GCC or use your platform's default version? (Y(Yes/build) / N(No/default))
Default option for your platform: ($DEFAULT_GCCBD_OPTION): " GCCBD_ANS

   case $GCCBD_ANS in
	[yY] ) echo  "Building GCC:" 1>&2;
	        build_gcc
		exit 0;;
	[nN] ) 	echo "Using default GCC" 1>&2;
		exit 0;;
	"" ) echo "Initiating default option ($DEFAULT_GCCBD_OPTION): " 1>&2;
	     if [ "$DEFAULT_GCCBD_OPTION" == "Yes" ]; then
	        echo "Building GCC" 1>&2;
	        build_gcc
	     	exit 0
	     elif [ "$DEFAULT_GCCBD_OPTION" == "No" ]; then
	        echo "Using default GCC:" 1>&2;
	        exit 0
	     fi ;;
	* ) echo "Please answer yes (Y/y) or no (N/n) " 1>&2;;
   esac


done
