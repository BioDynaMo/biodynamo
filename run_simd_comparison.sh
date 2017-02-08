#!/bin/bash
# script compiles different binaries (sse, avx with or without omp pragmas)
# and executes benchmarks
# script must be called from build directory

BIN=bdmp

# param1: optimization and simd flag (e.g. -O3 -mavx2)
# param2: binary name suffix
function compile {
  make clean >/dev/null ; rm CMakeCache.txt >/dev/null ; \
   cmake -Dtest=off -DCMAKE_CXX_FLAGS="$1" -DCMAKE_BUILD_TYPE=Debug .. >/tmp/${BIN}_log 2>&1  && \
   make -j5 >/tmp/${BIN}_log

  cp $BIN ${BIN}_$2
}

function commentOmpPragmas {
  sed -i.bak "s|#pragma omp|//#pragma omp|g" $1
  rm $1.bak
}

function uncommentOmpPragmas {
  sed -i.bak "s|//#pragma omp|#pragma omp|g" $1
  rm $1.bak
}

function withOmp {
  uncommentOmpPragmas ../src/dividing_cell_op.h
  uncommentOmpPragmas ../src/displacement_op.h
}

function noOmp {
  commentOmpPragmas ../src/dividing_cell_op.h
  commentOmpPragmas ../src/displacement_op.h
}

echo "SSE"
withOmp
compile "-O3" sse
sleep 1 && ./$BIN --detailed-scaling 5

noOmp
compile "-O3" sse_noomp
sleep 1 && ./$BIN 5

echo "AVX"
withOmp
compile "-O3 -mavx2" avx
sleep 1 && ./$BIN --detailed-scaling 5

noOmp
compile "-O3 -mavx2" avx_noomp
sleep 1 && ./$BIN 5

# revert to initial status
withOmp
