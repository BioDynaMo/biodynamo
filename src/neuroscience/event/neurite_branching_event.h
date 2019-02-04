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

#ifndef NEUROSCIENCE_EVENT_NEURITE_BRANCHING_EVENT_H_
#define NEUROSCIENCE_EVENT_NEURITE_BRANCHING_EVENT_H_

#include <array>
#include "core/event/event.h"

namespace bdm {
namespace experimental {
namespace neuroscience {

// clang-format off
/// \brief This event splits the current neurite element into two elements
/// and adds a new side branch as daughter right at the proximal half.
///
/// It is therefore a combination of SplitNeuriteElementEvent and
/// SideNeuriteExtensionEvent. The parameter names must be compatible to the
/// two mentioned events to enable code reuse.
/// (This event creates **two** new neurite elements.)
///
/// Here is the constructor to create a new proximal neurite, or side branch
/// for this event
/// NeuriteElementExt::NeuriteElementExt(const NeuriteBranchingEvent& event, TNeuriteElement* mother, uint64_t new_oid)
/// , and the corresponding event handler
/// NeuriteElementExt::EventHandler(const NeuriteBranchingEvent& event, TDaughter* left, TDaughter* right)
// clang-format on
struct NeuriteBranchingEvent : public Event {
  static const EventId kEventId;

  virtual ~NeuriteBranchingEvent() {};

  EventId GetId() const override { return kEventId; }

  /// the fraction of the total old length devoted to the
  /// distal half (should be between 0 and 1).
  double distal_portion_;
  /// length of the new side branch
  double length_;
  /// diameter of the new side branch
  double diameter_;
  /// direction of the new side branch.
  /// will be automatically corrected if not at least 45 degrees from the
  /// cylinder's axis.
  std::array<double, 3> direction_;
};

}  // namespace neuroscience
}  // namespace experimental
}  // namespace bdm

#endif  // NEUROSCIENCE_EVENT_NEURITE_BRANCHING_EVENT_H_
