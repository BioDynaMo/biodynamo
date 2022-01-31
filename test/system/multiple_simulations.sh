#!/bin/bash
# -----------------------------------------------------------------------------
#
# Copyright (C) 2022 CERN & University of Surrey for the benefit of the
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

set -e -x

tmp_dir=$(mktemp -d)
trap "rm -rf \"${tmp_dir}\"" EXIT

biodynamo demo multiple_simulations "${tmp_dir}"
cd "${tmp_dir}/multiple_simulations"
biodynamo run

# Check if ParaView was enabled for this BioDynaMo installation
set +e
bdm-config --config | grep -i paraview
rc_pv=$?
set -e
if [ $rc_pv -ne 0 ]; then
  exit 0
fi

# check if ParaView files exists
# first simulation
[ -f output/multiple_simulations/simulation_info.json ]
[ -f output/multiple_simulations/multiple_simulations.pvsm ]
[ -f output/multiple_simulations/Cell-0_0.vtu ]
[ -f output/multiple_simulations/Cell-0.pvtu ]
[ -f output/multiple_simulations/Cell-1_0.vtu ]
[ -f output/multiple_simulations/Cell-1.pvtu ]
[ -f output/multiple_simulations/Cell-2_0.vtu ]
[ -f output/multiple_simulations/Cell-2.pvtu ]
[ -f output/multiple_simulations/Cell-3_0.vtu ]
[ -f output/multiple_simulations/Cell-3.pvtu ]
[ -f output/multiple_simulations/Cell-4_0.vtu ]
[ -f output/multiple_simulations/Cell-4.pvtu ]

# second simulation
[ -f output/multiple_simulations1/simulation_info.json ]
[ -f output/multiple_simulations1/multiple_simulations1.pvsm ]
[ -f output/multiple_simulations1/Cell-0_0.vtu ]
[ -f output/multiple_simulations1/Cell-0.pvtu ]
[ -f output/multiple_simulations1/Cell-1_0.vtu ]
[ -f output/multiple_simulations1/Cell-1.pvtu ]
[ -f output/multiple_simulations1/Cell-2_0.vtu ]
[ -f output/multiple_simulations1/Cell-2.pvtu ]
[ -f output/multiple_simulations1/Cell-3_0.vtu ]
[ -f output/multiple_simulations1/Cell-3.pvtu ]
[ -f output/multiple_simulations1/Cell-4_0.vtu ]
[ -f output/multiple_simulations1/Cell-4.pvtu ]
