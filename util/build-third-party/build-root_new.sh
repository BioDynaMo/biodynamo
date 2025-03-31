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
  This script builds ROOT.
  The archive will be stored in BDM_PROJECT_DIR/build/root.tar.gz
Arguments:
  \$1 BDM_PROJECT_DIR" 1>&2;
  exit 2
fi

ROOT_VERSION_TGZ=root_v6.30.02


PYV2=`echo $PYVERS | cut -d . -f 1-2`

BDM_PROJECT_DIR=$1


# import util functions
. $BDM_PROJECT_DIR/util/installation/common/util.sh

ROOT_INSTALL_DIR=$BDM_PROJECT_DIR/third_party/root

ROOT_TGZ_FILE=$BDM_PROJECT_DIR/third_party/$ROOT_VERSION_TGZ.source.tar.gz

build_root(){
BDM_OS=$(DetectOs2)
PYTH=""
FEATURES=""
brewpy=no

if [ $BDM_OS == "osx" ]; then
  brewpy=yes
  if [ $brewpy == "yes" ]; then
  	 	 PYTH="$(brew --prefix)/bin/python${PYV2}"
	else
  		 export PYENV_ROOT=$(brew --prefix)/opt/.pyenv
  		 eval "$(pyenv init -)"
  		 pyenv shell $PYVERS
  		 PYTH="`pyenv which python`"
	fi
	FEATURES="\
 -Dmacos_native=YES \
 -Dclad=OFF \
 -Dxrootd=OFF \
 -Dbuiltin_fftw3=ON \
 -Dbuiltin_freetype=ON \
 -Dbuiltin_ftgl=ON \
 -Dbuiltin_glew=ON \
 -Dbuiltin_gsl=ON \
 -Dbuiltin_lz4=ON \
 -Dbuiltin_lzma=ON \
 -Dbuiltin_openssl=OFF \
 -Dbuiltin_xrootd=OFF \
 -Dbuiltin_pcre=ON \
 -Dbuiltin_tbb=ON \
 -Dbuiltin_unuran=ON \
 -Dbuiltin_xxhash=ON \
 -Dbuiltin_zlib=ON \
 -Dbuiltin_zstd=ON"
else
	FEATURES="\
 -Dclad=OFF \
 -Dxrootd=OFF \
 -Dbuiltin_fftw3=ON \
 -Dbuiltin_freetype=ON \
 -Dbuiltin_ftgl=ON \
 -Dbuiltin_glew=ON \
 -Dbuiltin_gsl=ON \
 -Dbuiltin_lz4=ON \
 -Dbuiltin_lzma=ON \
 -Dbuiltin_openssl=OFF \
 -Dbuiltin_xrootd=OFF \
 -Dbuiltin_pcre=ON \
 -Dbuiltin_tbb=ON \
 -Dbuiltin_unuran=ON \
 -Dbuiltin_xxhash=ON \
 -Dbuiltin_zlib=ON \
 -Dbuiltin_zstd=ON"

	PYTH="`pyenv which python`"
fi
DEST_DIR=$BDM_PROJECT_DIR/build/build-third-party/build_root
rm -rf $DEST_DIR
rm -rf $ROOT_INSTALL_DIR
mkdir -p $DEST_DIR
mkdir -p $ROOT_INSTALL_DIR
if ! [ -e $ROOT_TGZ_FILE ]; then
  curl -o $ROOT_TGZ_FILE https://root.cern/download/$ROOT_VERSION_TGZ.source.tar.gz
fi
tar -xvzf $ROOT_TGZ_FILE -C $DEST_DIR --strip-components=1
cd $DEST_DIR
if [ $ROOT_VERSION_TGZ=="root_v6.30.02" ]; then
  patch -p1 < $BDM_PROJECT_DIR/third_party/root63002.patch
fi
mkdir obj
cd obj
echo $PYTH
cmake $FEATURES -DCMAKE_CXX_STANDARD=17 -DCMAKE_BUILD_TYPE=Release  -DPYTHON_EXECUTABLE="$PYTH" -DCMAKE_INSTALL_PREFIX=$ROOT_INSTALL_DIR ..
cmake --build . --parallel $(CPUCount) --target install
#source $ROOT_INSTALL_DIR/bin/thisroot.sh
return
}

SPECIFIC_BDM_OS=$(DetectOs)

DEFAULT_ROOTBD_OPTION="Yes"

if echo "$SPECIFIC_OSES_SUPPORTED" | grep -Eiq "$SPECIFIC_BDM_OS" ;  then
	DEFAULT_ROOTBD_OPTION="No"
fi

while true; do
	read -p "Do you want to build or download ROOT? (Y(Yes/build) / N(No/download))
Default option for your platform: ($DEFAULT_ROOTBD_OPTION): " ROOTBD_ANS

   case $ROOTBD_ANS in
	[yY] ) echo  "Building ROOT:" 1>&2;
	        build_root
		exit 0;;
	[nN] ) 	echo "Downloading ROOT:" 1>&2;
		exit 1;;
	"" ) echo "Initiating default option ($DEFAULT_ROOTBD_OPTION): " 1>&2;
	     if [ "$DEFAULT_ROOTBD_OPTION" == "Yes" ]; then
	        echo "Building ROOT:" 1>&2;
	        build_root
	     	exit 0
	     elif [ "$DEFAULT_ROOTBD_OPTION" == "No" ]; then
	        echo "Downloading ROOT:" 1>&2;
	        exit 1
	     fi ;;
	* ) echo "Please answer yes (Y/y) or no (N/n) " 1>&2;;
   esac


done
