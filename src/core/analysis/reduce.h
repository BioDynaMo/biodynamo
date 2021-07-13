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
/// TODO
template <typename T, typename TResult = T>
class Reducer : public Functor<void, Agent*> {
 public:
  Reducer() {
    tl_results_.resize(ThreadInfo::GetInstance()->GetMaxThreads());
    for (auto& el : tl_results_) {
      el = T();
    }
  }

  Reducer(void(agent_function)(Agent*, T*),
          T (*reduce_partial_results)(const SharedData<T>&),
          TResult (*post_process)(TResult) = nullptr)
      : agent_function_(agent_function),
        reduce_partial_results_(reduce_partial_results),
        post_process_(post_process) {
    tl_results_.resize(ThreadInfo::GetInstance()->GetMaxThreads());
    for (auto& el : tl_results_) {
      el = T();
    }
  }

  virtual ~Reducer() {}

  void operator()(Agent* agent) override {
    auto tid = ThreadInfo::GetInstance()->GetMyThreadId();
    agent_function_(agent, &(tl_results_[tid]));
  }

  virtual TResult GetResult() {
    auto combined = static_cast<TResult>(reduce_partial_results_(tl_results_));
    if (post_process_) {
      return post_process_(combined);
    }
    return combined;
  }

 protected:
  SharedData<T> tl_results_;  //!

 private:
  void (*agent_function_)(Agent*, T*) = nullptr;                 //!
  T (*reduce_partial_results_)(const SharedData<T>&) = nullptr;  //!
  TResult (*post_process_)(TResult) = nullptr;                   //!
  BDM_CLASS_DEF(Reducer, 1)
};

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
/// The optional argument `filter` allows to reduce only a subset of
/// all agents.
template <typename T>
inline T Reduce(Simulation* sim, Functor<void, Agent*, T*>& agent_functor,
                Functor<T, const SharedData<T>&>& reduce_partial_results,
                Functor<bool, Agent*>* filter = nullptr) {
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
  rm->ForEachAgentParallel(actual_agent_func, filter);
  //   combine thread-local results
  return reduce_partial_results(tl_results);
}

// -----------------------------------------------------------------------------
// TODO
template <typename TResult = uint64_t>
struct Counter : public Reducer<uint64_t, TResult> {
 public:
  Counter(bool (*condition)(Agent*), TResult (*post_process)(TResult) = nullptr)
      : Reducer<uint64_t, TResult>(),
        condition_(condition),
        post_process_(post_process) {}
  virtual ~Counter() {}

  void operator()(Agent* agent) override {
    if (condition_(agent)) {
      auto tid = ThreadInfo::GetInstance()->GetMyThreadId();
      this->tl_results_[tid]++;
    }
  }

  TResult GetResult() override {
    SumReduction<uint64_t> sum;
    auto combined = static_cast<TResult>(sum(this->tl_results_));
    if (post_process_) {
      return post_process_(combined);
    }
    return combined;
  }

 private:
  bool (*condition_)(Agent*) = nullptr;         //!
  TResult (*post_process_)(TResult) = nullptr;  //!
  BDM_CLASS_DEF_OVERRIDE(Counter, 1)
};

/// Counts the number of agents for which `condition` evaluates to true.
/// Let's assume we want to count all infected agents in a virus spreading
/// simulation.
/// \code
/// auto is_infected = L2F([](Agent* a) {
///   return bdm_static_cast<Person*>(a)->state_ == State::kInfected;
/// });
/// auto num_infected = Count(sim, is_infected));
/// \endcode
/// The optional argument `filter` allows to count only a subset of
/// all agents.
inline uint64_t Count(Simulation* sim, Functor<bool, Agent*>& condition,
                      Functor<bool, Agent*>* filter = nullptr) {
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
  rm->ForEachAgentParallel(actual_agent_func, filter);
  //   combine thread-local results
  SumReduction<uint64_t> sum;
  return sum(tl_results);
}

}  // namespace experimental
}  // namespace bdm

#endif  // CORE_ANALYSIS_REDUCE_H_
