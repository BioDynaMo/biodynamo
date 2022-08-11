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

namespace bdm {
namespace experimental {

void SendReceive(
 MPI_Comm comm, 
 const std::vector<int>& neighbor_ranks,
 const std::unordered_map<int, std::pair<unsigned char*, uint64_t>>& send,
 std::unordered_map<int, std::pair<unsigned char*, uint64_t>>* receive
    ) {

  stk::CommNeighbors commNeighbors(comm, neighbor_ranks);

  int my_rank = commNeighbors.parallel_rank();

  // send
  for (int proc : neighbor_ranks) {
    if (proc != my_rank) {
      stk::CommBuffer& proc_buff = commNeighbors.send_buffer(proc);
      // FIXME avoid copy
      auto* dest = proc_buff.raw_buffer();
      std::memcpy(dest, send[proc].first, sned[proc].second);
    }
  }

  commNeighbors.communicate();

  // receive
  for (int procFromWhichDataIsReceived = 0;
       procFromWhichDataIsReceived < numProcs; procFromWhichDataIsReceived++) {
    if (procFromWhichDataIsReceived != me) {
      stk::CommBuffer& dataReceived =
          commNeighbors.recv_buffer(procFromWhichDataIsReceived);
      int numItemsReceived = 0;
      std::stringstream str;
      str << "P" << me << " data received from "
          << procFromWhichDataIsReceived << ": ";
      while (dataReceived.remaining()) {
        double val = -1;
        dataReceived.unpack(val);
        // EXPECT_EQ(100-procFromWhichDataIsReceived+numItemsReceived, val);
        str << val << ", ";
        numItemsReceived++;
      }
      int goldNumItemsReceived = procFromWhichDataIsReceived;
      EXPECT_EQ(goldNumItemsReceived, numItemsReceived);
      std::cout << str.str() << std::endl;
    }
  }
}

}  // namespace experimental
}  // namespace bdm
