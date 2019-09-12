#!/bin/bash

# In order to avoid sourcing the BioDynaMo environments between buliding and
# executing BDM binaries, we provide this wrapper. To be used as following:
# launcher.sh <regular_command_invocation>

SCRIPTPATH="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# We redirect to /dev/null to ignore the success message when we want to
# extract output in CMake (e.g. in Installation.cmake we want to obtain the
# version number).
source @CMAKE_INSTALL_ROOT@/bin/thisbdm.sh &> /dev/null
$@
