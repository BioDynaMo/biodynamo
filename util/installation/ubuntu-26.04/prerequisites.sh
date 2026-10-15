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

# Ubuntu 26.04 initially reuses the Ubuntu 24.04 binary dependencies.
set -euo pipefail

if [[ $# -ne 1 || ( "$1" != "all" && "$1" != "required" ) ]]; then
  echo "Usage: $0 <all|required>" >&2
  exit 1
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
. "$SCRIPT_DIR/environment.sh"

sudo apt-get update
mapfile -t packages < "$SCRIPT_DIR/package_list_required"
sudo apt-get install -y "${packages[@]}"

export PYENV_ROOT="$HOME/.pyenv"
export PATH="$PYENV_ROOT/bin:$PATH"
if [[ ! -x "$PYENV_ROOT/bin/pyenv" ]]; then
  git clone --depth 1 https://github.com/pyenv/pyenv.git "$PYENV_ROOT"
fi
eval "$(pyenv init --path)"
eval "$(pyenv init -)"
PYTHON_CONFIGURE_OPTS="--enable-shared" pyenv install --skip-existing "$BDM_PYTHON_VERSION"
pyenv shell "$BDM_PYTHON_VERSION"
# CMake 4 removes compatibility required by bundled third-party projects.
# Install into pyenv, without replacing the system CMake or system Python.
python -m pip install 'cmake==3.31.10'
pyenv rehash
python -c 'import ssl, bz2, ctypes, lzma, readline, sqlite3'

if [[ "$1" == "all" ]]; then
  mapfile -t packages < "$SCRIPT_DIR/package_list_extra"
  sudo apt-get install -y "${packages[@]}"
  python -m pip install -r "$SCRIPT_DIR/pip_packages.txt"
fi
