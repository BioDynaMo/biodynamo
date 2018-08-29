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

#ifndef NEUROSCIENCE_EVENT_SPLIT_NEURITE_ELEMENT_EVENT_H_
#define NEUROSCIENCE_EVENT_SPLIT_NEURITE_ELEMENT_EVENT_H_

#include "event/event.h"

namespace bdm {
namespace experimental {
namespace neuroscience {

// clang-format off
/// \brief Contains the parameters to split a neurite element into two segments.
///
/// This event splits a neurite element into two segments.
/// The neurite element that triggers the event becomes the distal one.
/// The new neurite element will be the proximal one.
///
/// Here is the constructor to create a new neurite element for this event
/// NeuriteElementExt::NeuriteElementExt(const SplitNeuriteElementEvent& event, TNeuriteElement* other)
/// and the corresponding event handler
/// NeuriteElementExt::EventHandler(const SplitNeuriteElementEvent& event, TNeurite* proximal)
// clang-format on
struct SplitNeuriteElementEvent {
  static const EventId kEventId;

  /// The fraction of the total old length devoted to the distal half
  /// (should be between 0 and 1).
  double distal_portion_;
};

}  // namespace neuroscience
}  // namespace experimental
}  // namespace bdm

#endif  // NEUROSCIENCE_EVENT_SPLIT_NEURITE_ELEMENT_EVENT_H_
