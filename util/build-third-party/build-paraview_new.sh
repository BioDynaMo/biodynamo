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
  This script builds ParaView.
  The archive will be stored in BDM_PROJECT_DIR/build/PARAVIEW.tar.gz
Arguments:
  \$1 BDM_PROJECT_DIR" 1>&2;
  exit 2
fi

PARAVIEW_VERSION_TXZ=ParaView-v5.9.0
CATALYST_VERSTON_TGZ=catalyst-for-paraview-5.9
BDM_PROJECT_DIR=$1


# import util functions
. $BDM_PROJECT_DIR/util/installation/common/util.sh

PARAVIEW_INSTALL_DIR=$BDM_PROJECT_DIR/third_party/paraview
CATALYST_TGZ_FILE=$BDM_PROJECT_DIR/third_party/$CATALYST_VERSTON_TGZ.tar.gz
PARAVIEW_TXZ_FILE=$BDM_PROJECT_DIR/third_party/$PARAVIEW_VERSION_TXZ.tar.xz

build_paraview(){
FEATURES=""
PYTH=""
BDM_OS=$(DetectOs2)
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

else
	PYTH="`pyenv which python`"
fi

DEST_DIR=$BDM_PROJECT_DIR/build/build-third-party/build_paraview



rm -rf $DEST_DIR
rm -rf $PARAVIEW_INSTALL_DIR
mkdir -p $DEST_DIR
mkdir -p $DEST_DIR/catalyst
mkdir -p $PARAVIEW_INSTALL_DIR
mkdir -p $PARAVIEW_INSTALL_DIR/catalyst
tar -xvzf $CATALYST_TGZ_FILE -C $DEST_DIR/catalyst --strip-components=1
tar -Jxvf $PARAVIEW_TXZ_FILE -C $DEST_DIR --strip-components=1
if [ $BDM_OS == "osx" ]; then
	if [ -d $BDM_PROJECT_DIR/build/third_party/qt ]; then
		export Qt5_DIR=$BDM_PROJECT_DIR/build/third_party/qt
	else
		export Qt5_DIR=$(brew --prefix qt5)/lib/cmake/Qt5
	fi
else 
	export Qt5_DIR=$BDM_PROJECT_DIR/build/third_party/qt
fi
cd $DEST_DIR/catalyst
mkdir obj
cd obj
cmake  -DCMAKE_INSTALL_PREFIX=$PARAVIEW_INSTALL_DIR/catalyst ..
make -j$(CPUCount)
make install
cd $DEST_DIR
mkdir obj
cd obj
export catalyst_DIR=$PARAVIEW_INSTALL_DIR/catalyst
cmake  -DENABLE_ospray:BOOL=ON -DENABLE_ospraymaterials:BOOL=ON -DENABLE_paraviewsdk:BOOL=ON -DPARAVIEW_USE_PYTHON=YES -DPARAVIEW_USE_CATALYST=YES -DPARAVIEW_ENABLE_CATALYST=YES -DPARAVIEW_ENABLE_NONESSENTIAL=YES -DPARAVIEW_USE_MPI=ON -DVTK_MODULE_ENABLE_ParaView_Catalyst=YES -DVTK_MODULE_ENABLE_ParaView_PythonCatalyst=YES -DPYTHON_EXECUTABLE="$PYTH" -DCMAKE_INSTALL_PREFIX=$PARAVIEW_INSTALL_DIR ..
make -j$(CPUCount)
make install
return
}


SPECIFIC_BDM_OS=$(DetectOs)

DEFAULT_PARAVIEWBD_OPTION="Yes"

if echo "$SPECIFIC_OSES_SUPPORTED" | grep -Eiq "$SPECIFIC_BDM_OS" ;  then
	DEFAULT_PARAVIEWBD_OPTION="No"
fi

while true; do
	read -p "Do you want to build, download or offer provided ParaView? (Y(Yes/build) / N(No/download))
Default option for your platform: ($DEFAULT_PARAVIEWBD_OPTION): " PARAVIEWBD_ANS

   case $PARAVIEWBD_ANS in
	[yY] ) echo  "Building ParaView:" 1>&2;
	        build_paraview
		exit 0;;
	[nN] ) 	echo "Downloading ParaView:" 1>&2;
		exit 1;;
	#[pP] )  echo "Extracting Provided ParaView:" 1>&2;
	      #  exit 3 ;;
	"" ) echo "Initiating default option ($DEFAULT_PARAVIEWBD_OPTION): " 1>&2;
	     if [ "$DEFAULT_PARAVIEWBD_OPTION" == "Yes" ]; then
	        echo "Building ParaView:" 1>&2;
	        build_paraview
	     	exit 0
	     elif [ "$DEFAULT_PARAVIEWBD_OPTION" == "No" ]; then
	        echo "Downloading ParaView:" 1>&2;
	        exit 1
	     elif [ "$DEFAULT_PARAVIEWBD_OPTION" == "Prov" ]; then
	       echo "Extracting Provided ParaView:" 1>&2;
	       exit 3
	     fi ;;
	* ) echo "Please answer yes (Y/y) or no (N/n) " 1>&2;;
   esac


done
