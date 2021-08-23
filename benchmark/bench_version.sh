#!/bin/bash

VERSION=`git log | awk '/commit/ {print $2}' | awk "NR==1{print;exit}"`
benchmark/bench_gen_plots.py benchmark/results.json $VERSION
