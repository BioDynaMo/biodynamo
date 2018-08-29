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

#include "backend.h"
#include "root_util.h"
#include "simulation_object_util.h"
#include "type_util.h"
#include "so_pointer.h"

namespace bdm {

/// Required to pass derived type to base class
template <template <typename, typename> class T>
struct Capsule {
  template <typename TCompileTimeParam, typename TDerived>
  using type = T<TCompileTimeParam, TDerived>;
};

/// Contains code required by all simulation objects
template <typename TCompileTimeParam, typename TDerived>
class SimulationObject {
 public:
  using MostDerived = typename TDerived::template type<TCompileTimeParam, TDerived>;;
  using MostDerivedSoPtr = SoPointer<TDerived>;

  SimulationObject() {}
  SimulationObject(const SimulationObject &) = default;
  virtual ~SimulationObject() {}

  uint32_t GetElementIdx() const { return element_idx_; }

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

  MostDerived *operator->() {
    return static_cast<MostDerived*>(this);
  }

  const MostDerived *operator->() const {
    return static_cast<const MostDerived*>(this);
  }

  // SimulationObject &operator=(const SimulationObject &) { return *this; }
  //
  // SimulationObject &operator=(SimulationObject &&other) {
  //   Base::operator=(std::move(other));
  //   return *this;
  // }

  protected:
   // array index of this object in the ResourceManager
   uint32_t element_idx_ = 0;

   ClassDef(SimulationObject, 1);
};

template <typename TCompileTimeParam, typename TDerived>
using SimulationObjectExt = SimulationObject<TCompileTimeParam, TDerived>;

}  // namespace bdm

#endif  // SIMULATION_OBJECT_H_
