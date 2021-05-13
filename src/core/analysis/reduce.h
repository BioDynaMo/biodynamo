// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & Newcastle University for the benefit of the
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

#ifndef CORE_ANALYSIS_REDUCE_H_
#define CORE_ANALYSIS_REDUCE_H_

#include <array>
#include <vector>

#include "core/agent/agent.h"
#include "core/container/shared_data.h"
#include "core/functor.h"
#include "core/util/thread_info.h"
#include "core/operation/reduction_op.h"
#include "core/resource_manager.h"

namespace bdm {
namespace experimental {

// -----------------------------------------------------------------------------
template <typename T>
T Reduce(Simulation* sim, Functor<void, Agent*, T*>& agent_functor,
         Functor<T, const SharedData<T>&>& reduce_partial_results) {
  // The thread-local (partial) results
  SharedData<T> tl_results;
  // initialize thread local data 
  tl_results.resize(ThreadInfo::GetInstance()->GetMaxThreads());
  for (auto& el : tl_results) {
    el = T();
  }
  
  // reduce
  //   execute agent functor in parallel  
  auto actual_agent_func = L2F([&](Agent* agent, AgentHandle){
    auto tid = ThreadInfo::GetInstance()->GetMyThreadId();
    agent_functor(agent, &(tl_results[tid]));
      });
  auto* rm = sim->GetResourceManager();
  rm->ForEachAgentParallel(actual_agent_func);
  //   combine thread-local results
  return reduce_partial_results(tl_results);
}

// -----------------------------------------------------------------------------
uint64_t Count(Simulation* sim, Functor<bool, Agent*>& condition) {
  // The thread-local (partial) results
  SharedData<uint64_t> tl_results;
  // initialize thread local data 
  tl_results.resize(ThreadInfo::GetInstance()->GetMaxThreads());
  for (auto& el : tl_results) {
    el = 0;
  }
  
  // reduce
  //   execute agent functor in parallel  
  auto actual_agent_func = L2F([&](Agent* agent, AgentHandle){
    auto tid = ThreadInfo::GetInstance()->GetMyThreadId();
    if (condition(agent)) {
      tl_results[tid]++;
    }
  });
  auto* rm = sim->GetResourceManager();
  rm->ForEachAgentParallel(actual_agent_func);
  //   combine thread-local results
  SumReduction<uint64_t> sum; 
  return sum(tl_results);
}

}  // namespace experimental
}  // namespace bdm

#endif  // CORE_ANALYSIS_REDUCE_H_
