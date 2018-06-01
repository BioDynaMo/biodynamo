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

# assumes working dir = build dir

# helpful tutorial
# https://gist.github.com/willprice/e07efd73fb7f13f917ea

# NB: DO NOT USE -x here. It would leak the github token to the terminal
set -e

if [ "$TRAVIS_BRANCH" != "master" ] || [ "$TRAVIS_OS_NAME" != "linux" ]; then
  exit 0
fi

# set-up git
git config --global user.email "bdmtravis@gmail.com"
git config --global user.name "BioDynaMo Travis-CI Bot"

BDM_BUILD_DIR=`pwd`

make doc

# checkout github pages dir, clean it and recreate folder structure
cd
git clone https://github.com/BioDynaMo/biodynamo.github.io.git
cd biodynamo.github.io
rm -rf *
mv $BDM_BUILD_DIR/doc/* .

# commit
git add -A
git commit -m "Update documentation (Travis build: $TRAVIS_BUILD_NUMBER)"

# push changes
set +x
git remote add origin-pages https://${GH_TOKEN}@github.com/BioDynaMo/biodynamo.github.io.git > /dev/null 2>&1
git push --quiet --set-upstream origin-pages master > /dev/null 2>&1
