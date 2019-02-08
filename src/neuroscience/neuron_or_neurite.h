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

#ifndef NEUROSCIENCE_NEURON_OR_NEURITE_H_
#define NEUROSCIENCE_NEURON_OR_NEURITE_H_

#include <array>
#include "core/sim_object/sim_object.h"
#include "core/sim_object/so_pointer.h"
// #include <set>
// #include <string>
// #include <unordered_map>
// #include <vector>
//
// #include "core/default_force.h"
// #include "core/shape.h"
// #include "core/util/log.h"
// #include "core/util/math.h"
// #include "core/util/random.h"
// #include "neuroscience/event/neurite_bifurcation_event.h"
// #include "neuroscience/event/neurite_branching_event.h"
// #include "neuroscience/event/new_neurite_extension_event.h"
// #include "neuroscience/event/side_neurite_extension_event.h"
// #include "neuroscience/event/split_neurite_element_event.h"
// #include "neuroscience/param.h"
//
namespace bdm {
namespace experimental {
namespace neuroscience {

class NeuriteElement;

/// The mother of a neurite element can either be a neuron or a neurite.
/// This class declares this interface.
class NeuronNeurite {
 public:
   // FIXME
  // const TNeuronSomaSoPtr& GetNeuronSomaSoPtr() const { return neuron_ptr_; }
  //
  // const TNeuriteElementSoPtr& GetNeuriteElementSoPtr() const {
  //   return neurite_ptr_;
  // }
  //
  // TNeuronSomaSoPtr& GetNeuronSomaSoPtr() { return neuron_ptr_; }
  //
  // TNeuriteElementSoPtr& GetNeuriteElementSoPtr() { return neurite_ptr_; }

  // bool IsNeuronSoma() const { return neuron_ptr_ != nullptr; }
  // bool IsNeuriteElement() const { return neurite_ptr_ != nullptr; }

  virtual const std::array<double, 3> GetPosition() const = 0;

  virtual std::array<double, 3> OriginOf(SoUid daughter_uid) const = 0;

  // virtual NeuronOrNeurite* GetMother() = 0;

  // FIXME
  // auto GetDaughterLeft() -> decltype(
  //     std::declval<TNeuriteElementSoPtr>()->GetDaughterLeft()) const {
  //   assert(IsNeuriteElement() &&
  //          "This function call is only allowed for a NeuriteElement");
  //   return neurite_ptr_->GetDaughterLeft();
  // }
  //
  // auto GetDaughterRight() -> decltype(
  //     std::declval<TNeuriteElementSoPtr>()->GetDaughterRight()) const {
  //   assert(IsNeuriteElement() &&
  //          "This function call is only allowed for a NeuriteElement");
  //   return neurite_ptr_->GetDaughterRight();
  // }
  //
  // auto GetRestingLength() -> decltype(
  //     std::declval<TNeuriteElementSoPtr>()->GetRestingLength()) const {
  //   assert(IsNeuriteElement() &&
  //          "This function call is only allowed for a NeuriteElement");
  //   return neurite_ptr_->GetRestingLength();
  // }

  virtual void UpdateDependentPhysicalVariables() = 0;

  virtual void UpdateRelative(const SoPointer<NeuriteElement>& old_rel,
                      const SoPointer<NeuriteElement>& new_rel) = 0;

  virtual void RemoveDaughter(const SoPointer<NeuriteElement>& mother) = 0;
};


}  // namespace neuroscience
}  // namespace experimental
}  // namespace bdm

#endif  // NEUROSCIENCE_NEURON_OR_NEURITE_H_
