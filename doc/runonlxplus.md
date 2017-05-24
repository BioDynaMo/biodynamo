# Run on CERN's lxplus Infrastructure

Instructions to build BioDyanaMo on CERN's lxplus infrastructure

## G++

```
export LD_LIBRARY_PATH=/afs/cern.ch/sw/lcg/external/gcc/5.2/x86_64-slc6-gcc52-opt/lib64:$LD_LIBRARY_PATH
cmake -DCMAKE_CXX_COMPILER=/afs/cern.ch/sw/lcg/external/gcc/5.2/x86_64-slc6-gcc52-opt/bin/g++ -DCMAKE_C_COMPILER=/afs/cern.ch/sw/lcg/external/gcc/5.2/x86_64-slc6-gcc52-opt/bin/gcc ..
make
```

## clang

```
export LD_LIBRARY_PATH=/afs/cern.ch/sw/lcg/external/llvm/3.9/x86_64-slc6/lib:$LD_LIBRARY_PATH
cmake -DCMAKE_C_COMPILER=/afs/cern.ch/sw/lcg/external/llvm/3.9/x86_64-slc6/bin/clang -DCMAKE_CXX_COMPILER=/afs/cern.ch/sw/lcg/external/llvm/3.9/x86_64-slc6/bin/clang++ ..
make
```

## icc
https://twiki.cern.ch/twiki/bin/view/Sandbox/GeorgiosBItzesSandbox
```
source /afs/cern.ch/sw/IntelSoftware/linux/all-setup.sh
cmake -DCMAKE_C_COMPILER=icc -DCMAKE_CXX_COMPILER=icpc ..
make
```
