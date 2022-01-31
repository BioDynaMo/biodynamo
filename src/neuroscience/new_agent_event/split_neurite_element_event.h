// -----------------------------------------------------------------------------
//
// Copyright (C) 2022 CERN & University of Surrey for the benefit of the
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

#ifndef NEUROSCIENCE_NEW_AGENT_EVENT_SPLIT_NEURITE_ELEMENT_EVENT_H_
#define NEUROSCIENCE_NEW_AGENT_EVENT_SPLIT_NEURITE_ELEMENT_EVENT_H_

#include "core/agent/new_agent_event.h"

namespace bdm {
namespace neuroscience {

/// \brief Contains the parameters to split a neurite element into two segments.
///
/// This event splits a neurite element into two segments.
/// The neurite element that triggers the event becomes the distal one.
/// The new neurite element will be the proximal one.
struct SplitNeuriteElementEvent : public NewAgentEvent {
  static const NewAgentEventUid kUid;

  explicit SplitNeuriteElementEvent(double distal_portion)
      : distal_portion(distal_portion) {}

  virtual ~SplitNeuriteElementEvent() {}

  NewAgentEventUid GetUid() const override { return kUid; }

  /// The fraction of the total old length devoted to the distal half
  /// (should be between 0 and 1).
  double distal_portion;
};

}  // namespace neuroscience
}  // namespace bdm

#endif  // NEUROSCIENCE_NEW_AGENT_EVENT_SPLIT_NEURITE_ELEMENT_EVENT_H_
