#!/bin/bash

VERSION=`git log | awk '/commit/ {print $2}' | awk "NR==1{print;exit}"`
./bench_gen_plots.py gbench/results.json $VERSION
