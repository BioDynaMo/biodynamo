#!/bin/bash

for i in 1 4 8
do
  for j in {1..10}
  do
    echo "../build/octree_bench ${i}"
    ../build/octree_bench 128 ${i} $1
  done
done

exit 0