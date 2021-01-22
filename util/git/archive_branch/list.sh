#!/bin/bash
# -----------------------------------------------------------------------------
#
# Copyright (C) 2021 CERN & Newcastle University for the benefit of the
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

# DESCRIPTION:
#   Lists all archived branches. Synchronizes with origin first
# USAGE:
#   list.sh
# EXAMPLE OUTPUT:
#   088d1..(shortened)..6a commit	refs/archive/lukas/commutative-pair-operation
#
#   archive branch name does not include 'refs/archive'
#   -> archived branch name is 'lukas/commutative-pair-operation'

# download all references
git fetch origin +refs/archive/*:refs/archive/*

git for-each-ref refs/archive
