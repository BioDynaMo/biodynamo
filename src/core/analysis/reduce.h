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
/// An interface for any type of implementation that wishes to
/// calculate a reduction over all agents
/// (e.g. counting, averaging, finding minimum and maximum values, etc.).\n
/// The benefit of this interface is that multiple reduction operations
/// can be combined to avoid iterating over all agents multiple times.\n
template <typename TResult>
struct Reducer : public Functor<void, Agent*> {
  virtual ~Reducer() = default;
  virtual TResult GetResult() = 0;
  /// Resets the internal state between calculations.
  virtual void Reset() = 0;
  virtual Reducer* NewCopy() const = 0;
  BDM_CLASS_DEF(Reducer, 1)
};

// -----------------------------------------------------------------------------
/// Generic implementation of a reduction.\n
/// Provides the functions to iterates over all agents executing the
/// `agent_function_` and updating a thread-local and therefore partial result.
/// The `reduce_partial_results_` attribute specifies how these partial results
/// should be combined into a single value.
/// Let's assume we want to sum up the `data` attribute of all agents.
/// \code
/// auto sum_data = [](Agent* agent, uint64_t* tl_result) {
///   *tl_result += bdm_static_cast<TestAgent*>(agent)->GetData();
/// };
/// auto combine_tl_results = [](const SharedData<uint64_t>& tl_results) {
///   uint64_t result = 0;
///   for (auto& el : tl_results) {
///     result += el;
///   }
///   return result;
/// };
/// GenericReducer<uint64_t> reducer(sum_data, combine_tl_results);
/// rm->ForEachAgentParallel(reducer);
/// auto result = reducer.GetResult();
/// \endcode
/// The optional argument `filter` allows to reduce only a subset of
/// all agents.
/// The benefit in comparison with `bdm::experimental::Reduce` is that
/// multiple counters can be combined and processed in one sweep over
/// all agents.
/// \see bdm::experimental::Reducer`
template <typename T, typename TResult = T>
class GenericReducer : public Reducer<TResult> {
 public:
  GenericReducer() { Reset(); }

  GenericReducer(void(agent_function)(Agent*, T*),
                 T (*reduce_partial_results)(const SharedData<T>&),
                 bool (*filter)(Agent*) = nullptr,
                 TResult (*post_process)(TResult) = nullptr)
      : agent_function_(agent_function),
        reduce_partial_results_(reduce_partial_results),
        filter_(filter),
        post_process_(post_process) {
    Reset();
    tl_results_.resize(ThreadInfo::GetInstance()->GetMaxThreads());
    for (auto& el : tl_results_) {
      el = T();
    }
  }

  virtual ~GenericReducer() = default;

  void operator()(Agent* agent) override {
    if (!filter_ || (filter_(agent))) {
      auto tid = ThreadInfo::GetInstance()->GetMyThreadId();
      agent_function_(agent, &(tl_results_[tid]));
    }
  }

  void Reset() override {
    tl_results_.resize(ThreadInfo::GetInstance()->GetMaxThreads());
    for (auto& el : tl_results_) {
      el = T();
    }
  }

  TResult GetResult() override {
    auto combined = static_cast<TResult>(reduce_partial_results_(tl_results_));
    if (post_process_) {
      return post_process_(combined);
    }
    return combined;
  }

  Reducer<TResult>* NewCopy() const override {
    return new GenericReducer(*this);
  }

 private:
  SharedData<T> tl_results_;                                     //!
  void (*agent_function_)(Agent*, T*) = nullptr;                 //!
  T (*reduce_partial_results_)(const SharedData<T>&) = nullptr;  //!
  bool (*filter_)(Agent*) = nullptr;                             //!
  TResult (*post_process_)(TResult) = nullptr;                   //!
  BDM_CLASS_DEF_OVERRIDE(GenericReducer, 1)
};

// The following custom streamer should be visible to rootcling for dictionary
// generation, but not to the interpreter!
#if (!defined(__CLING__) || defined(__ROOTCLING__)) && defined(USE_DICT)

// The custom streamer is needed because ROOT can't stream function pointers
// by default.
template <typename T, typename TResult>
inline void GenericReducer<T, TResult>::Streamer(TBuffer& R__b) {
  if (R__b.IsReading()) {
    R__b.ReadClassBuffer(GenericReducer::Class(), this);
    Long64_t l;
    R__b.ReadLong64(l);
    this->agent_function_ = reinterpret_cast<void (*)(Agent*, T*)>(l);
    R__b.ReadLong64(l);
    this->reduce_partial_results_ =
        reinterpret_cast<T (*)(const SharedData<T>&)>(l);
    R__b.ReadLong64(l);
    this->filter_ = reinterpret_cast<bool (*)(Agent*)>(l);
    R__b.ReadLong64(l);
    this->post_process_ = reinterpret_cast<TResult (*)(TResult)>(l);
  } else {
    R__b.WriteClassBuffer(GenericReducer::Class(), this);
    Long64_t l = reinterpret_cast<Long64_t>(this->agent_function_);
    R__b.WriteLong64(l);
    l = reinterpret_cast<Long64_t>(this->reduce_partial_results_);
    R__b.WriteLong64(l);
    l = reinterpret_cast<Long64_t>(this->filter_);
    R__b.WriteLong64(l);
    l = reinterpret_cast<Long64_t>(this->post_process_);
    R__b.WriteLong64(l);
  }
}

#endif  // !defined(__CLING__) || defined(__ROOTCLING__)

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
/// all agents.\n
/// NB: For better performance consider using `GenericReducer` instead.
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
/// Provides functions to count the number of agents for which the given
/// condition is true.\n
/// The following code example demonstrates how to count all agents
/// with `diameter < 5`.
/// \code
/// auto diam_lt_5 = [](Agent* agent) {
///   return agent->GetDiameter() < 5;
/// };
/// Counter<> counter(diam_lt_5);
/// rm->ForEachAgentParallel(counter);
/// auto result = counter.GetResult();
/// \endcode
/// The counting result can be post processed. Let's assume we want
/// to divide the counting result by two:
/// \code
/// auto diam_lt_5 = [](Agent* agent) {
///   return agent->GetDiameter() < 5;
/// };
/// auto post_process = [](uint64_t result) { return result / 2; };
/// Counter<> counter(diam_lt_5, post_process);
/// rm->ForEachAgentParallel(counter);
/// auto result = counter.GetResult();
/// \endcode
/// The benefit in comparison with `bdm::experimental::Count` is that
/// multiple counters can be combined and processed in one sweep over
/// all agents.
/// \see bdm::experimental::Reducer`
template <typename TResult = uint64_t>
struct Counter : public Reducer<TResult> {
 public:
  /// Required for IO
  Counter() { Reset(); }

  Counter(bool (*condition)(Agent*), TResult (*post_process)(TResult) = nullptr)
      : condition_(condition), post_process_(post_process) {
    Reset();
  }

  virtual ~Counter() = default;

  void Reset() override {
    tl_results_.resize(ThreadInfo::GetInstance()->GetMaxThreads());
    for (auto& el : tl_results_) {
      el = 0u;
    }
  }

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

  Reducer<TResult>* NewCopy() const override { return new Counter(*this); }

 private:
  SharedData<uint64_t> tl_results_;             //!
  bool (*condition_)(Agent*) = nullptr;         //!
  TResult (*post_process_)(TResult) = nullptr;  //!
  BDM_CLASS_DEF_OVERRIDE(Counter, 1)
};

// The following custom streamer should be visible to rootcling for dictionary
// generation, but not to the interpreter!
#if (!defined(__CLING__) || defined(__ROOTCLING__)) && defined(USE_DICT)

// The custom streamer is needed because ROOT can't stream function pointers
// by default.
template <typename TResult>
inline void Counter<TResult>::Streamer(TBuffer& R__b) {
  if (R__b.IsReading()) {
    R__b.ReadClassBuffer(Counter::Class(), this);
    Long64_t l;
    R__b.ReadLong64(l);
    this->condition_ = reinterpret_cast<bool (*)(Agent*)>(l);
    R__b.ReadLong64(l);
    this->post_process_ = reinterpret_cast<TResult (*)(TResult)>(l);
  } else {
    R__b.WriteClassBuffer(Counter::Class(), this);
    Long64_t l = reinterpret_cast<Long64_t>(this->condition_);
    R__b.WriteLong64(l);
    l = reinterpret_cast<Long64_t>(this->post_process_);
    R__b.WriteLong64(l);
  }
}

#endif  // !defined(__CLING__) || defined(__ROOTCLING__)

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
/// all agents.\n
/// NB: For better performance consider using `Counter` instead.
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
