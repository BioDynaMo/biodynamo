#!/bin/bash

# A wrapper for the BioDynaMo CLI to source
# the environment (mainly for DYLD_LIBRARY_PATH)
# to circumvent the OS X System Integrity Policy that
# strips away DYLD_LIBRARY_PATH for new proccesses
source $BDM_INSTALL_DIR/bin/thisbdm.sh -Q &> /dev/null

$@
