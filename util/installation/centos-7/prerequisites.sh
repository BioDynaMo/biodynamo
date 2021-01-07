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

# This script installs the required packages
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

sudo -v

sudo yum update -y

# Install repositories
sudo yum install -y https://centos7.iuscommunity.org/ius-release.rpm \
  epel-release centos-release-scl

# Install required packages
sudo yum install -y \
  $(cat $BDM_PROJECT_DIR/util/installation/centos-7/package_list_required)

if [ -n "${PYENV_ROOT}" ]; then
  unset PYENV_ROOT
fi

# If PyEnv is not installed, install it
if [ ! -f "$HOME/.pyenv/bin/pyenv" ]; then
  echo "PyEnv was not found. Installing now..."
  curl https://pyenv.run | bash
fi
export PATH="$HOME/.pyenv/bin:$PATH"
eval "$(pyenv init -)"

PYVERS=3.8.0

# If Python $PYVERS is not installed, install it
if [ ! -f  "$HOME/.pyenv/versions/$PYVERS/lib/libpython3.so" ]; then
  echo "Python $PYVERS was not found. Installing now..."
  /usr/bin/env PYTHON_CONFIGURE_OPTS="--enable-shared" pyenv install -f $PYVERS
fi
pyenv shell $PYVERS

# Install optional packages
if [ $1 == "all" ]; then
  PIP_PACKAGES="nbformat jupyter metakernel jupyterlab"
  # Don't install --user: the packages should end up in the PYENV_ROOT directory
  python -m pip install $PIP_PACKAGES
  # SBML integration
  sudo bash -c 'cat << EOF  > /etc/yum.repos.d/springdale-7-SCL.repo
[SCL-core]
name=Springdale SCL Base 7.6 - x86_64
mirrorlist=http://springdale.princeton.edu/data/springdale/SCL/7.6/x86_64/mirrorlist
#baseurl=http://springdale.princeton.edu/data/springdale/SCL/7.6/x86_64
gpgcheck=1
gpgkey=http://springdale.math.ias.edu/data/puias/7.6/x86_64/os/RPM-GPG-KEY-puias
EOF'
  sudo yum install -y --nogpgcheck \
    $(cat $BDM_PROJECT_DIR/util/installation/centos-7/package_list_extra)
fi

# Set up cmake alias such to be able to use it
# FIXME: this is will basically change permanently the system of the user
sudo alternatives --install /usr/local/bin/cmake cmake /usr/bin/cmake3 20 \
  --slave /usr/local/bin/ctest ctest /usr/bin/ctest3 \
  --slave /usr/local/bin/cpack cpack /usr/bin/cpack3 \
  --slave /usr/local/bin/ccmake ccmake /usr/bin/ccmake3 \
  --family cmake

exit 0
