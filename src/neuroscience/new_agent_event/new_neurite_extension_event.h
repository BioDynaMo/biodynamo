// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & Newcastle University for the benefit of the
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

#ifndef NEUROSCIENCE_NEW_AGENT_EVENT_NEW_NEURITE_EXTENSION_EVENT_H_
#define NEUROSCIENCE_NEW_AGENT_EVENT_NEW_NEURITE_EXTENSION_EVENT_H_

#include "core/agent/new_agent_event.h"

namespace bdm {
namespace neuroscience {

/// \brief Contains the parameters to extend a new neurite from a neuron soma.
///
/// The cell that triggers the event is the neuron soma.
/// The event creates a new neurite element.
struct NewNeuriteExtensionEvent : public NewAgentEvent {
  static const NewAgentEventUid kUid;

  NewNeuriteExtensionEvent(double diameter, double phi, double theta)
      : diameter(diameter), phi(phi), theta(theta) {}

  virtual ~NewNeuriteExtensionEvent() {}

  NewAgentEventUid GetUid() const override { return kUid; }

  /// diameter the diameter of the new neurite
  double diameter;
  /// phi azimuthal angle (spherical coordinates)
  double phi;
  /// theta polar angle (spherical coordinates)
  double theta;
};

}  // namespace neuroscience
}  // namespace bdm

#endif  // NEUROSCIENCE_NEW_AGENT_EVENT_NEW_NEURITE_EXTENSION_EVENT_H_
