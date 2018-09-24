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

#ifndef SIMULATION_OBJECT_H_
#define SIMULATION_OBJECT_H_

#include <algorithm>
#include <mutex>
#include <set>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "log.h"
#include "event/event.h"
#include "biology_module_util.h"

namespace bdm {

/// Contains code required by all simulation objects
class SimulationObject {
 public:
  SimulationObject() {}
  SimulationObject(const SimulationObject &) = default;
  virtual ~SimulationObject() {}

  // TODO used to "fall down" to most derived type
  virtual SimulationObject* New(const Event&, SimulationObject* other, uint64_t new_oid = 0) const = 0;

  virtual void EventHandler(const Event&, SimulationObject* so1) {}
  virtual void EventHandler(const Event&, SimulationObject* so1, SimulationObject* so2) {}

  /// NB: Cannot be used in the Constructur, because the ResourceManager`
  /// didn't initialize `element_idx_` yet.
  uint32_t GetElementIdx() const { return element_idx_; }

  uint64_t GetSoHandle() const { return element_idx_; }

  // assign the array index of this object in the ResourceManager
  void SetElementIdx(uint32_t element_idx) { element_idx_ = element_idx; }

  /// Empty default implementation to update references of simulation objects
  /// that changed its memory position.
  /// @param update_info vector index = type_id, map stores (old_index ->
  /// new_index)
  void UpdateReferences(
      const std::vector<std::unordered_map<uint32_t, uint32_t>> &update_info) {}

  /// Implementation to update a single reference if `reference.GetElementIdx()`
  /// is a key in `updates`.
  /// @tparam TReference type of the reference. Must have a `GetElementIdx` and
  ///         `SetElementIdx` method.
  /// @param reference reference whos `element_idx` will be updated
  /// @param updates map that contains the update information
  ///        (old_index -> new_index) for a specific simulation object type
  template <typename TReference>
  void UpdateReference(TReference *reference,
                       const std::unordered_map<uint32_t, uint32_t> &updates) {
    auto search = updates.find(reference->GetElementIdx());
    if (search != updates.end()) {
      reference->SetElementIdx(search->second);
    }
  }

  // FIXME remove virtual and move data member here
  virtual const std::array<double, 3>& GetPosition() const = 0;
  virtual double GetDiameter() const = 0;
  virtual void SetPosition(const std::array<double, 3>&) = 0;

  virtual std::array<double, 3> CalculateDisplacement(double squared_radius) const = 0;
  virtual void ApplyDisplacement(const std::array<double, 3>& displacement) = 0;

  uint32_t GetBoxIdx() const { return box_idx_; }

  void SetBoxIdx(uint32_t idx) { box_idx_ = idx; }

  /// Add a biology module to this cell
  /// @tparam TBiologyModule type of the biology module. Must be in the set of
  ///         types specified in `BiologyModules`
  void AddBiologyModule(BaseBiologyModule* module) {
    biology_modules_.push_back(module);
  }

  /// Execute all biology modules
  void RunBiologyModules() {
    for(auto* bm : biology_modules_) {
      bm->Run(this);
    }
  }

  // TODO
  /// Get all biology modules of this cell that match the given type.
  /// @tparam TBiologyModule  type of the biology module
  // std::vector<BaseBiologyModule*> GetBiologyModules()

  /// Grid box index
  uint32_t box_idx_;

  protected:
   // array index of this object in the ResourceManager
   uint32_t element_idx_ = 0;
   std::vector<BaseBiologyModule*> biology_modules_;

  //  ClassDef(SimulationObject, 1);
};

}  // namespace bdm

#endif  // SIMULATION_OBJECT_H_
