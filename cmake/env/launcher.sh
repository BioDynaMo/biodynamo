#!/bin/bash

# In order to avoid sourcing the BioDynaMo environments between building and
# executing BDM binaries, we provide this wrapper. To be used as follows:
# launcher.sh <regular_command_invocation>

SCRIPTPATH="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
export BDM_THISBDM_SILENT=true

# We redirect to /dev/null to ignore the success message when we want to
# extract output in CMake (e.g. in Installation.cmake we want to obtain the
# version number).
source @CMAKE_INSTALL_ROOT@/bin/thisbdm.sh

$@
