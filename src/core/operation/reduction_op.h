// -----------------------------------------------------------------------------
//
// Copyright (C) The BioDynaMo Project.
// All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#ifndef CORE_OPERATION_REDUCTION_OP_H_
#define CORE_OPERATION_REDUCTION_OP_H_

#include <array>
#include <vector>

#include "core/functor.h"
#include "core/operation/operation.h"
#include "core/operation/operation_registry.h"
#include "core/sim_object/sim_object.h"
#include "core/util/thread_info.h"

#define BDM_CACHE_LINE_SIZE 64

namespace bdm {

// This type avoids false sharing between threads
template <typename T>
using ThreadLocalResults =
    std::vector<std::array<T, BDM_CACHE_LINE_SIZE / sizeof(T)>>;

/// A template struct for any type of operation implementation that wishes to
/// implement a reduction operation (e.g. counting, averaging, finding minimum
/// and maximum values, etc.)
template <typename T>
struct ReductionOp : public SimObjectOperationImpl {
  BDM_OP_HEADER(ReductionOp);
  ReductionOp() {
    tl_results_.resize(ThreadInfo::GetInstance()->GetMaxThreads());
  }

  ~ReductionOp() {
    delete so_functor_;
    delete reduce_functor_;
  }

  // One element per timestep
  std::vector<T> results_;
  // The thread-local (partial) results
  ThreadLocalResults<T> tl_results_;

  // The functor containing the logic on what to execute for each sim object
  Functor<void, SimObject*, T*>* so_functor_ = nullptr;
  // The functor containing the logic on how to reduce the partial results into
  // a single result value of type T
  Functor<T, const ThreadLocalResults<T>&>* reduce_functor_ = nullptr;

  void SetUp() override {
    for (auto& arr : tl_results_) {
      arr[0] = T();
    }
  }

  void Initialize(Functor<void, SimObject*, T*>* so_functor,
                  Functor<T, const ThreadLocalResults<T>&>* reduce_functor) {
    so_functor_ = so_functor;
    reduce_functor_ = reduce_functor;
  }

  // This operator will be called for each simulation object in a parallel loop
  void operator()(SimObject* so) override {
    auto tid = ThreadInfo::GetInstance()->GetMyThreadId();
    (*so_functor_)(so, &(tl_results_[tid][0]));
  }

  // At the end of each timestep we collect the partial result of each thread
  // and reduce it to one single value
  void TearDown() override {
    results_.push_back((*reduce_functor_)(tl_results_));
  }
};

template <typename T>
struct SumReduction : public Functor<T, const ThreadLocalResults<T>&> {
  T operator()(const ThreadLocalResults<T>& tl_results) override {
    T result = 0;
    for (auto& el : tl_results) {
      // The other elements are padding to avoid false sharing
      result += el[0];
    }
    return result;
  }
};

}  // namespace bdm

#endif  // CORE_OPERATION_REDUCTION_OP_H_
