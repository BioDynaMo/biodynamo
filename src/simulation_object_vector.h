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

#ifndef SIMULATION_OBJECT_VECTOR_H_
#define SIMULATION_OBJECT_VECTOR_H_

#include <vector>
#include "resource_manager.h"  // SoHandle
#include "simulation.h"

namespace bdm {

/// Two dimensional vector. Holds one element of type `T` for each simulation
/// object in the simulation.
template <typename T, typename TSimulation = Simulation<>>
class SimulationObjectVector {
 public:
  /// NB: Elements will not be initilized.
  SimulationObjectVector() {
    auto* sim = TSimulation::GetActive();
    auto* rm = sim->GetResourceManager();
    data_.resize(rm->NumberOfTypes());
    size_.resize(rm->NumberOfTypes());
    Reserve();
  }

  /// Reserves enough memory to hold N instances of type T (N being the number
  /// of simulation objects in the simulation).
  /// e.g. ResourceManager has two types `A` and `B`. `A` has 10 elements
  /// and `B` 20. `data_[0]` corresponds to `A` and reserves 10 elements,
  /// while `data_[1]` corresponds to `B` and reserves 20 elements.
  /// NB: Elements will not be initilized.
  void Reserve() {
    clear();
    auto* sim = TSimulation::GetActive();
    auto* rm = sim->GetResourceManager();
    rm->ApplyOnAllTypes([&](auto* sim_objects, uint16_t type_idx) {
      data_[type_idx].reserve(sim_objects->size());
      size_[type_idx] = sim_objects->size();
    });
  }

  void clear() {  // NOLINT
    for (auto& vec : data_) {
      vec.clear();
    }
  }

  // Returns the number of element types
  size_t size() { return size_.size(); }  // NOLINT

  // Returns the number of elements of specified type
  size_t size(uint16_t type_idx) { return size_[type_idx]; }  // NOLINT

  const T& operator[](const SoHandle& handle) const {
    return data_[handle.GetTypeIdx()][handle.GetElementIdx()];
  }

  T& operator[](const SoHandle& handle) {
    return data_[handle.GetTypeIdx()][handle.GetElementIdx()];
  }

 private:
  /// one std::vector<T> for each type in ResourceManager
  std::vector<std::vector<T>> data_;
  std::vector<int> size_;
};

}  // namespace bdm

#endif  // SIMULATION_OBJECT_VECTOR_H_
