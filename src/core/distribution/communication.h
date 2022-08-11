// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & University of Surrey for the benefit of the
// BioDynaMo collaboration. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#ifndef CORE_DISTRIBUTION_COMMUNICATION_H__
#define CORE_DISTRIBUTION_COMMUNICATION_H__

#ifdef USE_DSE

#include <mpi.h>
#include <unordered_map>
#include <vector>
#include <set>

namespace bdm {

class Agent;

namespace experimental {

// -----------------------------------------------------------------------------
void SendReceive(MPI_Comm comm, const std::set<int>& neighbor_ranks, 
    const std::unordered_map<int, std::vector<Agent*>>& migrate_out, 
    std::unordered_map<int, std::vector<Agent*>>* migrate_in
    );

// -----------------------------------------------------------------------------
void SendReceive(
 MPI_Comm comm, 
 const std::set<int>& neighbor_ranks,
 const std::unordered_map<int, std::pair<char*, uint64_t>>& send,
 std::unordered_map<int, std::pair<char*, uint64_t>>* receive);

}  // namespace experimental
}  // namespace bdm


#endif   // USE_DSE

#endif  // CORE_DISTRIBUTION_COMMUNICATION_H__
