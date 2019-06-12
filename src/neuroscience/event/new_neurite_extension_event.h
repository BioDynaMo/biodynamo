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

#ifndef NEUROSCIENCE_EVENT_NEW_NEURITE_EXTENSION_EVENT_H_
#define NEUROSCIENCE_EVENT_NEW_NEURITE_EXTENSION_EVENT_H_

#include "core/event/event.h"

namespace bdm {
namespace experimental {
namespace neuroscience {

/// \brief Contains the parameters to extend a new neurite from a neuron soma.
///
/// The cell that triggers the event is the neuron soma.
/// The event creates a new neurite element.
struct NewNeuriteExtensionEvent : public Event {
  static const EventId kEventId;

  NewNeuriteExtensionEvent(double diameter, double phi, double theta)
      : diameter_(diameter), phi_(phi), theta_(theta) {}

  virtual ~NewNeuriteExtensionEvent() {}

  EventId GetId() const override { return kEventId; }

  /// diameter the diameter of the new neurite
  double diameter_;
  /// phi_ azimuthal angle (spherical coordinates)
  double phi_;
  /// theta_ polar angle (spherical coordinates)
  double theta_;
};

}  // namespace neuroscience
}  // namespace experimental
}  // namespace bdm

#endif  // NEUROSCIENCE_EVENT_NEW_NEURITE_EXTENSION_EVENT_H_
