#!/bin/bash

if [[ $# -ne 1 ]]; then
	
	echo "ERROR: Wrong number of arguments.
	Description: 
		This Script builds Libroadrunner.
	Arguments:
		\$1 BDM_PROJECT_DIR" 1>&2;
	exit 2
fi


BDM_PROJECT_DIR=$1


. $BDM_PROJECT_DIR/util/installation/common/util.sh

LLRR_INSTALL_DIR=$BDM_PROJECT_DIR/third_party/install_roadrunner

build_libroadrunner(){
echo $BDM_PROJECT_DIR
DEST_DIR=$BDM_PROJECT_DIR/build/build-third-party/build_roadrunner
rm -rf $DEST_DIR
rm -rf $LLRR_INSTALL_DIR
mkdir -p $DEST_DIR
mkdir -p $LLRR_INSTALL_DIR
cd $DEST_DIR
git clone https://github.com/sys-bio/llvm-13.x.git
cd llvm-13.x
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX="$LLRR_INSTALL_DIR/llvm" -DCMAKE_BUILD_TYPE="Release" ../llvm
cmake --build . --parallel $(CPUCount) --target install --config Release


cd $DEST_DIR
git clone https://github.com/sys-bio/libroadrunner-deps.git --recurse-submodules 
cd libroadrunner-deps
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX="$LLRR_INSTALL_DIR/deps-install-Release" -DCMAKE_BUILD_TYPE="Release" ..
cmake --build . --parallel $(CPUCount) --target install --config Release


cd $DEST_DIR
git clone https://github.com/sys-bio/roadrunner.git
cd roadrunner
mkdir build-release
cd build-release
cmake -DCMAKE_INSTALL_PREFIX="$LLRR_INSTALL_DIR/roadrunner-Release" \
    -DLLVM_INSTALL_PREFIX="$LLRR_INSTALL_DIR/llvm" \
    -DRR_DEPENDENCIES_INSTALL_PREFIX="$LLRR_INSTALL_DIR/deps-install-Release" \
    -DCMAKE_BUILD_TYPE="Release" \
    -DRR_USE_CXX11=OFF -DUSE_TR1_CXX_NS=ON \
    -DBUILD_TESTS=ON ..
cmake --build . --parallel $(CPUCount) --target install --config Release
ctest --verbose .

exit 0
}


build_libroadrunner
