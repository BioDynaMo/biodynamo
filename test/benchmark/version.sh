#!/bin/bash

VERSION=`git log | awk '/commit/ {print $2}' | awk "NR==1{print;exit}"`
../test/benchmark/gen_benchmark_plots.py benchmark/results.json $VERSION