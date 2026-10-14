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

# Keep the Python 3.9 ABI used by ROOT and ParaView, with OpenSSL 3 support.
export BDM_PYTHON_VERSION=3.9.25
# BioDynaMo itself builds with the platform's default compiler, same as on
# Ubuntu 22.04/24.04. gcc-11 only needs to be installed on disk (see
# package_list_required) so that ROOT's Cling interpreter finds the matching
# headers it was built against; forcing CC/CXX to gcc-11 here would compile
# BioDynaMo itself with an older libstdc++ ABI than the reused ROOT/ParaView
# archives require, breaking the final link.
