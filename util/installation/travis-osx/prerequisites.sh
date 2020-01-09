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

brew update
brew style
brew update-reset

# Install and upgrade required packages
brew install libomp tbb open-mpi git \
python python@2 llvm wget cmake || true

brew upgrade python cmake || true

# temporary fix to create correct python3 symlinks
# TODO: remove when we rely fully on Python 3
brew unlink python && brew link python
ln -sf /usr/local/bin/python3 /usr/local/bin/python

# Install the optional packages
if [ $1 == "all" ]; then
    PIP_PACKAGES="nbformat jupyter metakernel"
    pip2 install --user $PIP_PACKAGES

    # Jupyter relies on tornado for logging, but the latest tornado (from version
    # 6) is not compatible with Python 2. So we downgrade to 5.1.1
    pip2 uninstall tornado -y
    pip2 install --user tornado==5.1.1
    brew install doxygen graphviz lcov gcovr || true
fi
