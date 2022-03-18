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

  SideNeuriteExtensionEvent(real_t length, real_t diameter,
                            const Real3 direction)
      : length(length), diameter(diameter), direction(direction) {}

  virtual ~SideNeuriteExtensionEvent() = default;

  NewAgentEventUid GetUid() const override { return kUid; }

  /// length of the new branch
  real_t length;
  /// diameter of the new branch
  real_t diameter;
  /// direction of the new branch
  Real3 direction;
};

}  // namespace neuroscience
}  // namespace bdm

#endif  // NEUROSCIENCE_NEW_AGENT_EVENT_SIDE_NEURITE_EXTENSION_EVENT_H_
