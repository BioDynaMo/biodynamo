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

#include "core/distribution/communication.h"

#include "stk_util/parallel/CommNeighbors.hpp"

#ifdef USE_DSE

#include "core/multi_simulation/mpi_helper.h"

namespace bdm {
namespace experimental {

// -----------------------------------------------------------------------------
void SendReceive(
    MPI_Comm comm, const std::set<int>& neighbor_ranks,
    const std::unordered_map<int, std::vector<Agent*>>& migrate_out,
    std::unordered_map<int, std::vector<Agent*>>* migrate_in) {
  // serialize
  std::vector<MPIObject*> tmp;
  std::unordered_map<int, std::pair<char*, uint64_t>> out;
  for (auto& el : migrate_out) {
    auto* message = new MPIObject();
    message->WriteObject(&el.second);
    out.insert({el.first, {message->Buffer(), message->BufferSize()}});
    tmp.push_back(message);
  }

  std::unordered_map<int, std::pair<char*, uint64_t>> in;
  SendReceive(comm, neighbor_ranks, out, &in);

  // deserialize
  for (auto& el : in) {
    MPIObject message(el.second.first, el.second.second);
    auto* agent_vec = reinterpret_cast<std::vector<Agent*>*>(
        message.ReadObject(message.GetClass()));
    // FIXME another copy
    (*migrate_in)[el.first] = *agent_vec;
  }

  // free memory
  for (auto* message : tmp) {
    delete message;
  }
}

// -----------------------------------------------------------------------------
void SendReceive(
    MPI_Comm comm, const std::set<int>& neighbor_ranks,
    const std::unordered_map<int, std::pair<char*, uint64_t>>& send,
    std::unordered_map<int, std::pair<char*, uint64_t>>* receive) {
  std::vector<int> neighbor_ranks_vec(neighbor_ranks.begin(),
                                      neighbor_ranks.end());
  stk::CommNeighbors commNeighbors(comm, neighbor_ranks_vec);

  int my_rank = commNeighbors.parallel_rank();

  // send
  for (int proc : neighbor_ranks) {
    if (proc != my_rank) {
      stk::CommBufferV& proc_buff = commNeighbors.send_buffer(proc);
      const auto& data = send.at(proc);
      // FIXME avoid copy
      for (uint64_t i = 0; i < data.second; ++i) {
        proc_buff.pack<char>(data.first[i]);
      }
    }
  }

  commNeighbors.communicate();

  // receive
  for (int proc : neighbor_ranks) {
    stk::CommBufferV& proc_buff = commNeighbors.recv_buffer(proc);

    auto size = proc_buff.size_in_bytes();
    auto& pair = (*receive)[proc];
    pair.second = size;
    pair.first = new char[size];

    // FIXME copy
    std::memcpy(pair.first, proc_buff.raw_buffer(), size);
  }
}

}  // namespace experimental
}  // namespace bdm

#endif  // USE_DSE
