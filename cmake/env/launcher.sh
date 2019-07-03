#!/bin/bash

SCRIPTPATH="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

source @CMAKE_BIODYNAMO_ROOT@/biodynamo-env.sh
$@