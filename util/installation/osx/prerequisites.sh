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

BDM_PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../../.."

brew update
brew style
brew update-reset

# Install and upgrade required packages
brew install \
  $(cat $BDM_PROJECT_DIR/util/installation/osx/package_list_required) || true
brew upgrade cmake || true

# Install Python 3.6.9 environment
eval "$(pyenv init -)"
env PYTHON_CONFIGURE_OPTS="--enable-shared" pyenv install 3.6.9
pyenv shell 3.6.9

# Install the optional packages
if [ $1 == "all" ]; then
    PIP_PACKAGES="nbformat jupyter metakernel"
    pip install --user $PIP_PACKAGES
    brew install \
      $(cat $BDM_PROJECT_DIR/util/installation/osx/package_list_extra) || true
fi
