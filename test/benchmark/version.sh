#!/bin/bash

VERSION=`git log | awk '/commit/ {print $2}' | awk "NR==1{print;exit}"`
./gen_benchmark_plots.py results.json $VERSION