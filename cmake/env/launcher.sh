#!/bin/bash

SCRIPTPATH="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

source @CMAKE_INSTALL_ROOT@/bin/thisbdm.sh
$@
