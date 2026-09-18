#!/bin/bash

get_script_dir() {
  local SOURCE="${BASH_SOURCE[0]}"
  # Resolve symlinks
  while [ -L "$SOURCE" ]; do
    local DIR
    DIR="$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )"
    SOURCE="$(readlink "$SOURCE")"
    # If symlink was relative, resolve relative to symlink base dir
    [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE"
  done
  # Get final absolute directory
  cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd
}

# Determine the directory of this script
BDM_SCRIPT_DIR=$(get_script_dir)
cd $BDM_SCRIPT_DIR

# Build and run the simulation
export OMP_NUM_THREADS=1
bdm run

echo "Start rendering..."
pvbatch $BDM_SCRIPT_DIR/pv_script.py --screenshots

