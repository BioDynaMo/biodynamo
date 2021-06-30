#!/bin/bash

BDM_SCRIPT_DIR=$(readlink -e $(dirname "${BASH_SOURCE[0]}"))

cd $BDM_SCRIPT_DIR
biodynamo build
cd build

echo "Start simulation..."
export OMP_NUM_THREADS=1
./pyramidal_cell --config ../config.json --inline-config="{ \"bdm::Param\": { \"export_visualization\": true }}"

echo "Start rendering..."
pvbatch $BDM_SCRIPT_DIR/render.py --screenshots

