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

#ifndef CORE_CONTAINER_SIM_OBJECT_VECTOR_H_
#define CORE_CONTAINER_SIM_OBJECT_VECTOR_H_

#include <vector>
#include "core/resource_manager.h"  // SoHandle
#include "core/simulation.h"

namespace bdm {

/// Two dimensional vector. Holds one element of type `T` for each simulation
/// object in the simulation.
template <typename T>
class SimObjectVector {
 public:
  /// NB: Elements will not be initilized.
  SimObjectVector() {
    data_.resize(thread_info_->GetNumaNodes());
    size_.resize(thread_info_->GetNumaNodes());
    reserve();
  }

  /// Reserves enough memory to hold N instances of type T (N being the number
  /// of simulation objects in the simulation).
  /// e.g. ResourceManager has two types `A` and `B`. `A` has 10 elements
  /// and `B` 20. `data_[0]` corresponds to `A` and reserves 10 elements,
  /// while `data_[1]` corresponds to `B` and reserves 20 elements.
  /// NB: Elements will not be initilized.
  void reserve() {  // NOLINT
    clear();
    auto* sim = Simulation::GetActive();
    auto* rm = sim->GetResourceManager();
    for (int n = 0; n < thread_info_->GetNumaNodes(); n++) {
      auto num_sos = rm->GetNumSimObjects(n);
      if (data_[n].capacity() < num_sos) {
        data_[n].reserve(num_sos * 1.5);
      }
      size_[n] = num_sos;
    }
  }

  void clear() {  // NOLINT
    for (auto& el : size_) {
      el = 0;
    }
    for (auto& vec : data_) {
      vec.clear();
    }
  }

  // Returns the number of elements of specified type
  size_t size(uint16_t numa_node) { return size_[numa_node]; }  // NOLINT

  const T& operator[](const SoHandle& handle) const {
    return data_[handle.GetNumaNode()][handle.GetElementIdx()];
  }

  T& operator[](const SoHandle& handle) {
    return data_[handle.GetNumaNode()][handle.GetElementIdx()];
  }

 private:
  /// one std::vector<T> for each numa node
  std::vector<std::vector<T>> data_;
  std::vector<size_t> size_;
  ThreadInfo* thread_info_ = ThreadInfo::GetInstance();
};

}  // namespace bdm

#endif  // CORE_CONTAINER_SIM_OBJECT_VECTOR_H_
