#!/bin/bash

biodynamo build
cd build

./epidemiology --config ../measles.json --inline-config="{ \"bdm::Param\": { \"visualization_engine\": \"rooteve\", \"visualization_interval\": 100, \"export_visualization\": true }}" --repeat=1

#echo "Start rendering..."
#pvbatch ../render.py --screenshots --raytracing  --name measles
