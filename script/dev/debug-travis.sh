#!/bin/bash

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
