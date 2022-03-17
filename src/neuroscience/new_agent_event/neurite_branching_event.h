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

#ifndef NEUROSCIENCE_NEW_AGENT_EVENT_NEURITE_BRANCHING_EVENT_H_
#define NEUROSCIENCE_NEW_AGENT_EVENT_NEURITE_BRANCHING_EVENT_H_

#include <array>
#include "core/agent/new_agent_event.h"

namespace bdm {
namespace neuroscience {

/// \brief This event splits the current neurite element into two elements
/// and adds a new side branch as daughter right at the proximal half.
///
/// It is therefore a combination of SplitNeuriteElementEvent and
/// SideNeuriteExtensionEvent. The parameter names must be compatible to the
/// two mentioned events to enable code reuse.
/// (This event creates **two** new neurite elements.)
struct NeuriteBranchingEvent : public NewAgentEvent {
  static const NewAgentEventUid kUid;

  NeuriteBranchingEvent(real_t distal_portion, real_t length, real_t diameter,
                        const Real3 direction)
      : distal_portion(distal_portion),
        length(length),
        diameter(diameter),
        direction(direction) {}

  virtual ~NeuriteBranchingEvent() {}

  NewAgentEventUid GetUid() const override { return kUid; }

  /// the fraction of the total old length devoted to the
  /// distal half (should be between 0 and 1).
  real_t distal_portion;
  /// length of the new side branch
  real_t length;
  /// diameter of the new side branch
  real_t diameter;
  /// direction of the new side branch.
  /// will be automatically corrected if not at least 45 degrees from the
  /// cylinder's axis.
  Real3 direction;
};

}  // namespace neuroscience
}  // namespace bdm

#endif  // NEUROSCIENCE_NEW_AGENT_EVENT_NEURITE_BRANCHING_EVENT_H_
