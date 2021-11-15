#!/bin/bash

# Determine the directory of this script
BDM_SCRIPT_DIR=$(readlink -e $(dirname "${BASH_SOURCE[0]}"))
cd $BDM_SCRIPT_DIR

# Build and run the simulation
export OMP_NUM_THREADS=1
bdm run

echo "Start rendering..."
pvbatch $BDM_SCRIPT_DIR/pv_script.py --screenshots

