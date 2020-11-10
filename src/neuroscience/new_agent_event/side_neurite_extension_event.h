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

#ifndef NEUROSCIENCE_NEW_AGENT_EVENT_SIDE_NEURITE_EXTENSION_EVENT_H_
#define NEUROSCIENCE_NEW_AGENT_EVENT_SIDE_NEURITE_EXTENSION_EVENT_H_

#include "core/agent/new_agent_event.h"

namespace bdm {
namespace neuroscience {

/// \brief Contains the parameters to add a side neurite element.
///
/// This event adds a side neurite (daughter right) to the neurite element
/// that triggered the event.
struct SideNeuriteExtensionEvent : public NewAgentEvent {
  static const NewAgentEventUid kUid;

  SideNeuriteExtensionEvent(double length, double diameter,
                            const Double3 direction)
      : length_(length), diameter_(diameter), direction_(direction) {}

  virtual ~SideNeuriteExtensionEvent() {}

  NewAgentEventUid GetUid() const override { return kUid; }

  /// length of the new branch
  double length_;
  /// diameter of the new branch
  double diameter_;
  /// direction of the new branch
  Double3 direction_;
};

}  // namespace neuroscience
}  // namespace bdm

#endif  // NEUROSCIENCE_NEW_AGENT_EVENT_SIDE_NEURITE_EXTENSION_EVENT_H_
