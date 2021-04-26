#!/bin/bash

BDM_SCRIPT_DIR=$(readlink -e $(dirname "${BASH_SOURCE[0]}"))

cd $BDM_SCRIPT_DIR
biodynamo build
cd build

./epidemiology --config ../measles.json --beta 0.06719 --gamma 0.00521
cp output/result.svg output/measles.svg

