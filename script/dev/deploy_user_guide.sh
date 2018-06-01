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

# Generates website files for user guide and pushes changes to branch "gh-pages"
# Run this script after you made changes to the user guide's source code
# Website: https://biodynamo.github.io/biodynamo/

# get path of this script
pushd `dirname $0` > /dev/null
SCRIPTPATH=`pwd`

# go to user guide directory (contains mkdocs.yml)
pushd $SCRIPTPATH/../../doc/user_guide > /dev/null

# deploy mkdocs to github pages
mkdocs gh-deploy

rm -rf site

# go back to original directory
popd > /dev/null
popd > /dev/null
