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
  This script builds QT.
  The archive will be stored in BDM_PROJECT_DIR/build/QT.tar.gz
Arguments:
  \$1 BDM_PROJECT_DIR" 1>&2;
  exit 2
fi

QT_VERSION_TXZ=qtbase-everywhere-opensource-src-5.15.16

BDM_PROJECT_DIR=$1


# import util functions
. $BDM_PROJECT_DIR/util/installation/common/util.sh

QT_INSTALL_DIR=$BDM_PROJECT_DIR/third_party/qt

QT_TXZ_FILE=$BDM_PROJECT_DIR/third_party/$QT_VERSION_TXZ.tar.xz
declare -a QT_MODULE_TXZ=("qtsvg-everywhere-opensource-src-5.15.16" "qtxmlpatterns-everywhere-opensource-src-5.15.16" "qttools-everywhere-opensource-src-5.15.16" "qtwayland-everywhere-opensource-src-5.15.16" "qtmultimedia-everywhere-opensource-src-5.15.16" "qtmqtt-everywhere-opensource-src-5.15.16" "qtknx-everywhere-opensource-src-5.15.16" "qtimageformats-everywhere-opensource-src-5.15.16")

build_qt(){
BDM_OS=$(DetectOs2)

DEST_DIR=$BDM_PROJECT_DIR/build/build-third-party/build_qt
rm -rf $DEST_DIR
rm -rf $QT_INSTALL_DIR
mkdir -p $DEST_DIR
tar -Jxvf $QT_TXZ_FILE -C $DEST_DIR --strip-components=1
cd $DEST_DIR
sed -i 's@/usr/local/Qt-$\$\[QT_VERSION]@/usr/local/Qt@g' configure.pri
sed -i "s@/usr/local/Qt@$QT_INSTALL_DIR@g" configure.pri
./configure -xcb -xcb-xlib -bundled-xcb-xinput -opensource -confirm-license
make -j$(CPUCount)
make install
QMAKE_BIN=$QT_INSTALL_DIR/bin/qmake
arraylength=${#QT_MODULE_TXZ[@]}
for((i=0; i<${arraylength}; i++));
 do
	mkdir $BDM_PROJECT_DIR/build/build-third-party/build_qt_module_$i
	tar -Jxvf $BDM_PROJECT_DIR/third_party/qtmodules/${QT_MODULE_TXZ[$i]}.tar.xz -C $BDM_PROJECT_DIR/build/build-third-party/build_qt_module_$i --strip-components=1
	cd $BDM_PROJECT_DIR/build/build-third-party/build_qt_module_$i
	$QMAKE_BIN .
	make -j$(CPUCount)
	make install
done
return
}

SPECIFIC_BDM_OS=$(DetectOs)

DEFAULT_QTBD_OPTION="Yes"

if echo "$SPECIFIC_OSES_SUPPORTED" | grep -Eiq "$SPECIFIC_BDM_OS" ;  then
	DEFAULT_ROOTBD_OPTION="No"
fi

while true; do
	read -p "Do you want to build or download QT? (Y(Yes/build) / N(No/download))
Default option for your platform: ($DEFAULT_QTBD_OPTION): " QTBD_ANS

   case $QTBD_ANS in
	[yY] ) echo  "Building QT:" 1>&2;
	        build_qt
		exit 0;;
	[nN] ) 	echo "Downloading QT:" 1>&2;
		exit 1;;
	"" ) echo "Initiating default option ($DEFAULT_QTBD_OPTION): " 1>&2;
	     if [ "$DEFAULT_QTBD_OPTION" == "Yes" ]; then
	        echo "Building QT:" 1>&2;
	        build_qt
	     	exit 0
	     elif [ "$DEFAULT_QTBD_OPTION" == "No" ]; then
	        echo "Downloading QT:" 1>&2;
	        exit 1
	     fi ;;
	* ) echo "Please answer yes (Y/y) or no (N/n) " 1>&2;;
   esac


done
