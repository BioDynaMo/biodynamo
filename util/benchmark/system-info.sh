#!/bin/bash

## This script prints out general system information.

# Exit on error
set -e

# Using this function instead of "set -x", because it would also print empty
# `echo ""`
exe() { echo -e "\n\$ $@" ; "$@" ; }

exe lsb_release -a
exe uname -a
exe free -h 
exe lscpu

