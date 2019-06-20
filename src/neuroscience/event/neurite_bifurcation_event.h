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

#include "core/container/math_array.h"
#include "core/event/event.h"

namespace bdm {
namespace experimental {
namespace neuroscience {

/// \brief Contains the parameters to bifurcate a growth cone.
///
/// This event is only possible for terminal neurite segments.
/// It creates **two** new neurite elements and assigns it to daughter left
/// and daughter right of the neurite element that triggered the event
/// (=mother).
struct NeuriteBifurcationEvent : public Event {
  static const EventId kEventId;

  NeuriteBifurcationEvent(double length, double diameter_l, double diameter_r,
                          const Double3& direction_l,
                          const Double3& direction_r)
      : length_(length),
        diameter_left_(diameter_l),
        diameter_right_(diameter_r),
        direction_left_(direction_l),
        direction_right_(direction_r) {}

  virtual ~NeuriteBifurcationEvent() {}

  EventId GetId() const override { return kEventId; }

  /// length of new branches
  double length_;
  /// diameter  of new branch left
  double diameter_left_;
  /// diameter of new branch right
  double diameter_right_;
  /// direction branch right
  /// NB: direction will be corrected if it is pointing backward.
  Double3 direction_left_;
  /// direction branch left
  /// NB: direction will be corrected if it is pointing backward.
  Double3 direction_right_;
};

}  // namespace neuroscience
}  // namespace experimental
}  // namespace bdm

#endif  // NEUROSCIENCE_EVENT_NEURITE_BIFURCATION_EVENT_H_
