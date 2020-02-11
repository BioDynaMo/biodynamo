#!/bin/bash
# -----------------------------------------------------------------------------
#
# Copyright (C) The BioDynaMo Project.
# All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
#
# See the LICENSE file distributed with this work for details.
# See the NOTICE file distributed with this work for additional information
# regarding copyright ownership.
#
# -----------------------------------------------------------------------------

# This script installs the required packages for ubuntu 18.04
if [[ $# -ne 1 ]]; then
  echo "ERROR: Wrong number of arguments.
Description:
    This script installs the prerequisites of BioDynaMo, but not BioDynaMo
    itself. Script install.sh installs both prerequisites and BioDynaMo.
Arguments:
    <install_type>  all/required. If all is specified, then this script
                    will install all the prerequisites."
  exit 1
fi

set -e

BDM_PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../../.."

# This will avoid tzdata package from requesting user interaction (tzdata is a
# dependency of one of the prerequisites of PyEnv)
if ! [ -L /etc/localtime ]; then
  sudo ln -fs /usr/share/zoneinfo/Europe/Berlin /etc/localtime
fi

# use ubuntu-16.04 prerequisites script
. $BDM_PROJECT_DIR/util/installation/ubuntu-16.04/prerequisites.sh $1
