#!/bin/bash
# -----------------------------------------------------------------------------
#
# Copyright (C) 2021 CERN & University of Surrey for the benefit of the
# BioDynaMo collaboration. All Rights Reserved.
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

BDM_PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../../../.."

sudo -v


sudo yum update -y

# Install repositories

sudo yum config-manager --enable *codeready*

sudo yum config-manager --enable *crb*

sudo yum config-manager --enable *powertools*

sudo subscription-manager repos --enable *powertools*

sudo subscription-manager repos --enable *codeready*

sudo yum update -y

sudo yum install -y --nogpgcheck --skip-broken epel-release

sudo yum update -y


# Install required packages

if echo $(yum --version) | awk '{print $1}' | grep -Eiq 'dnf5' ; then

  sudo dnf install -y --nogpgcheck --skip-unavailable \
    $(cat $BDM_PROJECT_DIR/util/installation/rhel/common/package_list_required)

else

  sudo yum install -y --nogpgcheck --skip-broken \
    $(cat $BDM_PROJECT_DIR/util/installation/rhel/common/package_list_required)

fi

curl -L -O https://github.com/Kitware/CMake/releases/download/v3.19.3/cmake-3.19.3-Linux-x86_64.sh
chmod +x cmake-3.19.3-Linux-x86_64.sh
sudo ./cmake-3.19.3-Linux-x86_64.sh --skip-license --prefix=/usr/local

if [ -n "${PYENV_ROOT}" ]; then
  unset PYENV_ROOT
fi

# If PyEnv is not installed, install it
if [ ! -f "$HOME/.pyenv/bin/pyenv" ]; then
  echo "PyEnv was not found. Installing now..."
  curl https://pyenv.run | bash
fi
export PYENV_ROOT="$HOME/.pyenv"
export PATH="$PYENV_ROOT/bin:$PATH"
eval "$(pyenv init --path)"
eval "$(pyenv init -)"
pyenv update

PYVERS=3.9.1

# If Python $PYVERS is not installed, install it
if [ ! -f  "$HOME/.pyenv/versions/$PYVERS/lib/libpython3.so" ]; then
  echo "Python $PYVERS was not found. Installing now..."
  /usr/bin/env PYTHON_CONFIGURE_OPTS="--enable-shared" pyenv install -f $PYVERS
fi
pyenv shell $PYVERS

# Install optional packages
if [ $1 == "all" ]; then
  python -m pip install --upgrade pip
  PIP_PACKAGES="markupsafe==2.0.1 nbformat jupyter metakernel jupyterlab nbformat==5.4.0 nbconvert==6.5.3 nbclient==0.6.6 setuptools cython"
  # Don't install --user: the packages should end up in the PYENV_ROOT directory
  python -m pip install $PIP_PACKAGES
  # SBML integration
  if echo $(yum --version) | awk '{print $1}' | grep -Eiq 'dnf5' ; then

      sudo dnf install -y --nogpgcheck --skip-unavailable \
        $(cat $BDM_PROJECT_DIR/util/installation/rhel/common/package_list_extra)

  else

      sudo yum install -y --nogpgcheck --skip-broken \
        $(cat $BDM_PROJECT_DIR/util/installation/rhel/common/package_list_extra)

  fi

fi
. /etc/profile.d/modules.sh
module load mpi/openmpi-x86_64

gcc --version | awk '/gcc/ && ($3+0)>11{print "WARNING: Selected GCC Version is greater than 11, This can cause installation problems. Installing an older gcc version is recomended."}'

exit 0
