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

#include "core/event/event.h"

namespace bdm {
namespace experimental {
namespace neuroscience {

/// \brief Contains the parameters to split a neurite element into two segments.
///
/// This event splits a neurite element into two segments.
/// The neurite element that triggers the event becomes the distal one.
/// The new neurite element will be the proximal one.
struct SplitNeuriteElementEvent : public Event {
  static const EventId kEventId;

  explicit SplitNeuriteElementEvent(double distal_portion)
      : distal_portion_(distal_portion) {}

  virtual ~SplitNeuriteElementEvent() {}

  EventId GetId() const override { return kEventId; }

  /// The fraction of the total old length devoted to the distal half
  /// (should be between 0 and 1).
  double distal_portion_;
};

}  // namespace neuroscience
}  // namespace experimental
}  // namespace bdm

#endif  // NEUROSCIENCE_EVENT_SPLIT_NEURITE_ELEMENT_EVENT_H_
