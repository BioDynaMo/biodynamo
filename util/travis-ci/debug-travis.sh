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

# This script requests a debug build in Travis CI. The build is determined by
# the Travis Job ID, which can be found in the job description of the failing
# build. The authorization token can be found in your Travis settings.

if [[ $# -ne 2 ]] ; then
    echo 'Error: invalid arguments provided. First argument is Travis Job ID. Second argument is authorization token.'
    exit 1
fi

curl -s -X POST \
   -H "Content-Type: application/json" \
   -H "Accept: application/json" \
   -H "Travis-API-Version: 3" \
   -H "Authorization: token $2" \
   -d '{ "quiet": true }' \
   https://api.travis-ci.org/job/$1/debug
