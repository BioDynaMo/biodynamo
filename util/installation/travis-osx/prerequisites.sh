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
brew install libomp tbb open-mpi git llvm wget cmake pyenv readline xz || true
brew upgrade cmake python || true
# necessary since symlinking broke with brew on travis since 3.6.7_1 release
brew link --overwrite python

# On Travis CI pyenv is already installed, so we need to unset the following
unset PYENV_ROOT

eval "$(pyenv init -)" # this enables pyenv for the current shell
env PYTHON_CONFIGURE_OPTS="--enable-shared" pyenv install 3.6.9
pyenv shell 3.6.9

# Install the optional packages
if [ $1 == "all" ]; then
  PIP_PACKAGES="nbformat jupyter metakernel"
  pip install --user $PIP_PACKAGES
  brew install doxygen graphviz lcov gcovr || true
fi
