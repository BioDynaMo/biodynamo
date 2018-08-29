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

#ifndef NEUROSCIENCE_EVENT_NEURITE_BIFURCATION_EVENT_H_
#define NEUROSCIENCE_EVENT_NEURITE_BIFURCATION_EVENT_H_

#include <array>
#include "event/event.h"

namespace bdm {
namespace experimental {
namespace neuroscience {

// clang-format off
/// \brief Contains the parameters to bifurcate a growth cone.
///
/// This event is only possible for terminal neurite segments.
/// It creates **two** new neurite elements and assigns it to daughter left
/// and daughter right of the neurite element that triggered the event
/// (=mother).
///
/// Here is the constructor to create a new neurite element for this event
/// NeuriteElementExt::NeuriteElementExt(const NeuriteBifurcationEvent& event, TNeuriteElement* mother, uint64_t new_oid)
/// and the corresponding event handler
/// NeuriteElementExt::EventHandler(const NeuriteBifurcationEvent& event, TDaughter* left, TDaughter* right)
// clang-format on
struct NeuriteBifurcationEvent {
  static const EventId kEventId;

  /// length of new branches
  double length_;
  /// diameter  of new branch left
  double diameter_left_;
  /// diameter of new branch right
  double diameter_right_;
  /// direction branch right
  /// NB: direction will be corrected if it is pointing backward.
  std::array<double, 3> direction_left_;
  /// direction branch left
  /// NB: direction will be corrected if it is pointing backward.
  std::array<double, 3> direction_right_;
};

}  // namespace neuroscience
}  // namespace experimental
}  // namespace bdm

#endif  // NEUROSCIENCE_EVENT_NEURITE_BIFURCATION_EVENT_H_
