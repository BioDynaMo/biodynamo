#!/bin/bash

for j in {1..10}
do
echo "../build/octree_bench ${i}"
../build/octree_bench 128 8 $1
done

exit 0