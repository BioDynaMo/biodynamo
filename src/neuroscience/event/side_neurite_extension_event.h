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

#ifndef NEUROSCIENCE_EVENT_SIDE_NEURITE_EXTENSION_EVENT_H_
#define NEUROSCIENCE_EVENT_SIDE_NEURITE_EXTENSION_EVENT_H_

#include "core/event/event.h"

namespace bdm {
namespace experimental {
namespace neuroscience {

// clang-format off
/// \brief Contains the parameters to add a side neurite element.
///
/// This event adds a side neurite (daughter right) to the neurite element
/// that triggered the event.
///
/// Here is the constructor to create a new neurite element for this event
/// NeuriteElementExt::NeuriteElementExt(const SideNeuriteExtensionEvent& event, TNeuriteElement* mother)
/// and the corresponding event handler
/// NeuriteElementExt::EventHandler(const SideNeuriteExtensionEvent& event, TDaughter* right)
// clang-format on
struct SideNeuriteExtensionEvent : public Event {
  static const EventId kEventId;

  virtual ~SideNeuriteExtensionEvent() {};

  EventId GetId() const override { return kEventId; }

  /// length of the new branch
  double length_;
  /// diameter of the new branch
  double diameter_;
  /// direction of the new branch
  std::array<double, 3> direction_;
};

}  // namespace neuroscience
}  // namespace experimental
}  // namespace bdm

#endif  // NEUROSCIENCE_EVENT_SIDE_NEURITE_EXTENSION_EVENT_H_
