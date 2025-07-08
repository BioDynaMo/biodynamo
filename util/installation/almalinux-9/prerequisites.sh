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

BDM_PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../../.."

sudo -v

sudo dnf -y update

# Install required packages
sudo dnf -y --enablerepo=crb install epel-release
sudo dnf -y --enablerepo=crb install \
  $(cat $BDM_PROJECT_DIR/util/installation/almalinux-9/package_list_required)

# Install optional packages
if [ $1 == "all" ]; then
  python3 -m pip install --upgrade pip
  PIP_PACKAGES="markupsafe==2.0.1 nbformat jupyter metakernel jupyterlab nbformat==5.4.0 nbconvert==6.5.3 nbclient==0.6.6"
  # Don't install --user: the packages should end up in the PYENV_ROOT directory
  python3 -m pip install $PIP_PACKAGES
  # SBML integration
  #sudo bash -c 'cat << EOF  > /etc/yum.repos.d/springdale-7-SCL.repo
#[SCL-core]
#name=Springdale SCL Base 7.6 - x86_64
#mirrorlist=http://springdale.princeton.edu/data/springdale/SCL/7.6/x86_64/mirrorlist
##baseurl=http://springdale.princeton.edu/data/springdale/SCL/7.6/x86_64
#gpgcheck=1
#gpgkey=http://springdale.math.ias.edu/data/puias/7.6/x86_64/os/RPM-GPG-KEY-puias
#EOF'
  sudo dnf -y --enablerepo=crb install --nogpgcheck \
    $(cat $BDM_PROJECT_DIR/util/installation/almalinux-9/package_list_extra)
fi

exit 0
