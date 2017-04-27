#!/bin/bash

set -e -x

echo ${TRAVIS_OS_NAME}
biod=`pwd`

# update and install packages
if [ "$TRAVIS_OS_NAME" = "osx" ]; then
  sw_vers
  osx_vers=`sw_vers -productVersion | cut -d . -f1 -f2`
  if [ "$osx_vers" != "10.12" ]; then
     test_valgrind="-Dvalgrind=ON"
  fi
  brew update >& /dev/null
  brew install doxygen
  brew install valgrind
  #brew install cloc
  # get clang 3.9
  wget http://releases.llvm.org/3.9.0/clang+llvm-3.9.0-x86_64-apple-darwin.tar.xz 2> /dev/null
  tar xf clang+llvm-3.9.0-x86_64-apple-darwin.tar.xz > /dev/null
  export LLVMDIR="`pwd`/clang+llvm-3.9.0-x86_64-apple-darwin"
  export CC=$LLVMDIR/bin/clang
  export CXX=$LLVMDIR/bin/clang++
  export CXXFLAGS=-I$LLVMDIR/include
  export LDFLAGS=-L$LLVMDIR/lib
  export DYLD_LIBRARY_PATH=$LLVMDIR/lib:$DYLD_LIBRARY_PATH
  # get latest cmake
  wget https://cmake.org/files/v3.6/cmake-3.6.1-Darwin-x86_64.tar.gz 2> /dev/null
  tar zxf cmake-3.6.1-Darwin-x86_64.tar.gz > /dev/null
  export PATH="`pwd`/cmake-3.6.1-Darwin-x86_64/CMake.app/Contents/bin":$PATH:
fi

if [ "$TRAVIS_OS_NAME" = "linux" ]; then
  #sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
  #sudo add-apt-repository -y ppa:george-edison55/trusty-backports
  wget -O - http://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
  sudo apt-add-repository -y "deb http://apt.llvm.org/trusty/ llvm-toolchain-trusty-3.9 main"
  sudo apt-get update
  #sudo apt-get -y install gcc-5 g++-5 cmake cmake-data valgrind
  sudo apt-get -y install valgrind
  sudo apt-get -y install cloc
  sudo apt-get -y install clang-format-3.9
fi

# install ROOT
cd
if [ "$TRAVIS_OS_NAME" = "linux" ]; then
  wget https://cernbox.cern.ch/index.php/s/BbFptgxo2K565IS/download?path=%2F&files=root_v6.08.00-patches.Linux-ubuntu14-x86_64-gcc4.8.tar.gz.tar.gz 2> /dev/null
  tar zxf root_v6.08.00-patches.Linux-ubuntu14-x86_64-gcc4.8.tar.gz.tar.gz > /dev/null
else
  wget https://cernbox.cern.ch/index.php/s/BbFptgxo2K565IS/download?path=%2F&files=root_v6.08.00-patches.macosx64-10.12-clang80.tar.gz 2> /dev/null
  tar zxf root_v6.08.00-patches.macosx64-10.12-clang80.tar.gz > /dev/null
fi
cd root
. bin/thisroot.sh

# output compiler information
echo ${CXX}
${CXX} --version
${CXX} -v

cd $biod

# run following commands (cloc and clang-format) only on Linux
if [ "$TRAVIS_OS_NAME" = "linux" ]; then
  #cloc --vcs=git .
  cloc .
  if [ "$TRAVIS_PULL_REQUEST" != "false" ]; then
    BASE_COMMIT=$(git rev-parse $TRAVIS_BRANCH)
    echo "Running clang-format-3.9 against branch $TRAVIS_BRANCH, with hash $BASE_COMMIT"
    RESULT_OUTPUT="$(git-clang-format-3.9 --commit $BASE_COMMIT --diff --binary `which clang-format-3.9`)"
    if [ "$RESULT_OUTPUT" == "no modified files to format" ] || \
       [ "$RESULT_OUTPUT" == "clang-format did not modify any files" ]; then
      echo "clang-format passed."
    else
      echo "###### Code formatting failure ######"
      echo "clang-format failed."
      echo "To reproduce it locally please run git-clang-format-3.9 --commit $BASE_COMMIT --diff --binary \`which clang-format-3.9\`"
      echo "$RESULT_OUTPUT"
    fi
  fi
fi

# build biodynamo and run tests
mkdir build
cd build
cmake $test_valgrind .. && make
make check
