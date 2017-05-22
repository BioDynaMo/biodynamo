#!/bin/bash

for i in {100..5000..250}
do
  for j in {1..10}
  do
    echo "../build/octree_bench ${i}"
    ../build/octree_bench ${i}
  done
done

exit 0