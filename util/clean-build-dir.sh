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
   
if [ $# -ne 1 ]; then
  echo "ERROR: $0 expects one argument."
  echo "USAGE:"
  echo "  $0 path-to-build-dir"
  exit 1
fi

cd $1

# safety check:
# only accept directories that contain CMakeCache.txt and third_party
if ! [ -f "CMakeCache.txt" ] || ! [ -d "third_party" ] ; then
  echo "ERROR: The given directory ($1) does not contain a CMakeCache.txt file"
  echo "       nor a third_party directory. Therefore, this is likely not a"
  echo "       build directory. Aborting operation..."
  exit 1
fi

for f in *; do
  if [[ "$f" != "third_party" ]]; then
    rm -rf $f
  fi
done

for f in .ninja*; do
  rm -f $f
done

# remove file created in third_party/paraview/lib/cmake/paraview-5.5/Modules/
find . -name BDMGlyphFilterHierarchy.txt -exec rm -f {} \;
