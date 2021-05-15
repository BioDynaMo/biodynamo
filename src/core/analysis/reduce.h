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
#include "core/operation/reduction_op.h"
#include "core/resource_manager.h"
#include "core/util/thread_info.h"

namespace bdm {
namespace experimental {

// -----------------------------------------------------------------------------
/// Iterates over all agents executing the `agent_functor` and updating a
/// a thread-local and therefore partial result.
/// The second parameter specifies how these partial results should be combined
/// into a single value.
/// Let's assume we want to sum up the `data` attribute of all agents.
/// \code
/// auto sum_data = L2F([](Agent* agent, uint64_t* tl_result) {
///   *tl_result += bdm_static_cast<TestAgent*>(agent)->GetData();
/// });
/// SumReduction<uint64_t> combine_tl_results;
/// auto result = Reduce(sim, sum_data, combine_tl_results);
/// \endcode
template <typename T>
inline T Reduce(Simulation* sim, Functor<void, Agent*, T*>& agent_functor,
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
  auto actual_agent_func = L2F([&](Agent* agent, AgentHandle) {
    auto tid = ThreadInfo::GetInstance()->GetMyThreadId();
    agent_functor(agent, &(tl_results[tid]));
  });
  auto* rm = sim->GetResourceManager();
  rm->ForEachAgentParallel(actual_agent_func);
  //   combine thread-local results
  return reduce_partial_results(tl_results);
}

// -----------------------------------------------------------------------------
/// Counts the number of agents for which `condition` evaluates to true.
/// Let's assume we want to count all infected agents in a virus spreading
/// simulation.
/// \code
/// auto is_infected = L2F([](Agent* a) {
///   return bdm_static_cast<Person*>(a)->state_ == State::kInfected;
/// });
/// auto num_infected = Count(sim, is_infected));
/// \endcode
inline uint64_t Count(Simulation* sim, Functor<bool, Agent*>& condition) {
  // The thread-local (partial) results
  SharedData<uint64_t> tl_results;
  // initialize thread local data
  tl_results.resize(ThreadInfo::GetInstance()->GetMaxThreads());
  for (auto& el : tl_results) {
    el = 0;
  }

  // reduce
  //   execute agent functor in parallel
  auto actual_agent_func = L2F([&](Agent* agent, AgentHandle) {
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
