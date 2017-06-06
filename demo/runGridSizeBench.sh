#!/bin/bash

for i in {8..128..8}
do
  for j in {1..10}
  do
    echo "../build/octree_bench ${i}"
    ../build/octree_bench ${i} 8 $1
  done
done

exit 0