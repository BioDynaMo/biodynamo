#!/bin/bash

# Generates website files for user guide and pushes changes to branch "gh-pages"
# Run this script after you made changes to the user guide's source code
# Website: https://biodynamo.github.io/biodynamo/

# get path of this script
pushd `dirname $0` > /dev/null
SCRIPTPATH=`pwd`

# go to user guide directory (contains mkdocs.yml)
pushd $SCRIPTPATH/../../doc/user_guide > /dev/null

# deploy mkdocs to github pages
sudo mkdocs gh-deploy

# go back to original directory
popd > /dev/null
popd > /dev/null
