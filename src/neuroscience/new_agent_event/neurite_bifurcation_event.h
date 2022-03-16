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

#ifndef NEUROSCIENCE_NEW_AGENT_EVENT_NEURITE_BIFURCATION_EVENT_H_
#define NEUROSCIENCE_NEW_AGENT_EVENT_NEURITE_BIFURCATION_EVENT_H_

#include <array>

#include "core/agent/new_agent_event.h"
#include "core/container/math_array.h"

namespace bdm {
namespace neuroscience {

/// \brief Contains the parameters to bifurcate a growth cone.
///
/// This event is only possible for terminal neurite segments.
/// It creates **two** new neurite elements and assigns it to daughter left
/// and daughter right of the neurite element that triggered the event
/// (=mother).
struct NeuriteBifurcationEvent : public NewAgentEvent {
  static const NewAgentEventUid kUid;

  NeuriteBifurcationEvent(double length, double diameter_l, double diameter_r,
                          const Double3& direction_l,
                          const Double3& direction_r)
      : length(length),
        diameter_left(diameter_l),
        diameter_right(diameter_r),
        direction_left(direction_l),
        direction_right(direction_r) {}

  virtual ~NeuriteBifurcationEvent() = default;

  NewAgentEventUid GetUid() const override { return kUid; }

  /// length of new branches
  double length;
  /// diameter  of new branch left
  double diameter_left;
  /// diameter of new branch right
  double diameter_right;
  /// direction branch right
  /// NB: direction will be corrected if it is pointing backward.
  Double3 direction_left;
  /// direction branch left
  /// NB: direction will be corrected if it is pointing backward.
  Double3 direction_right;
};

}  // namespace neuroscience
}  // namespace bdm

#endif  // NEUROSCIENCE_NEW_AGENT_EVENT_NEURITE_BIFURCATION_EVENT_H_
