#!/bin/bash

BDM_SCRIPT_DIR=$(readlink -e $(dirname "${BASH_SOURCE[0]}"))

cd $BDM_SCRIPT_DIR
biodynamo build
cd build

./epidemiology --config ../measles.json --config ../plot-style.json
cp output/result.svg output/measles.svg

