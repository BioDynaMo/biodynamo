#!/bin/bash

BDM_SCRIPT_DIR=$(readlink -e $(dirname "${BASH_SOURCE[0]}"))

cd $BDM_SCRIPT_DIR
biodynamo build
cd build

./epidemiology --config ../measles.json --inline-config="{ \"bdm::Param\": { \"visualization_interval\": 100, \"export_visualization\": true }}" --repeat=1

echo "Start rendering..."
pvbatch $BDM_SCRIPT_DIR/render.py --screenshots --raytracing  --name measles

