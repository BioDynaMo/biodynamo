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

if [[ $# -ne 1 ]]; then
  echo "ERROR: No arguments expected
  Description:
    Run installation test inside a headless docker container."
  exit 1
fi

BDM_PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../.."
cd $BDM_PROJECT_DIR

# import util functions
. $BDM_PROJECT_DIR/util/installation/common/util.sh

# start x virtual framebuffer for headless environments.
export DISPLAY=:99.0
$BDM_PROJECT_DIR/util/xvfb-initd.sh start

test/installation-test.sh $1
RET_VAL=$?

$BDM_PROJECT_DIR/util/xvfb-initd.sh stop

# debug output for centos docker issue
# sometimes script inside docker container does not terminate
if [ $1 = "centos-7.6.1810" ]; then
  ps -ef
fi

exit $RET_VAL
