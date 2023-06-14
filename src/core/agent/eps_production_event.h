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

#ifndef CORE_AGENT_EPS_PRODUCTION_EVENT_H_
#define CORE_AGENT_EPS_PRODUCTION_EVENT_H_

#include "core/agent/new_agent_event.h"
#include "core/real_t.h"

namespace bdm {

/// \brief Contains the parameters to perform a cell division.
///
/// A cell division divides a mother cell in two daughter cells.\n
/// When the mother cell divides, by definition:\n
/// 1) the mother cell becomes the 1st daughter cell\n
/// 2) the new cell becomes the 2nd daughter cell
///
/// The cell that triggers the event is the mother.
struct EpsProductionEvent : public NewAgentEvent {
  static const NewAgentEventUid kUid;

  EpsProductionEvent(real_t volume_fraction, real_t phi, real_t theta)
      : volume_fraction(volume_fraction), phi(phi), theta(theta) {}

  virtual ~EpsProductionEvent() = default;

  NewAgentEventUid GetUid() const override { return kUid; }

  /// volume_ratio the ratio (Volume daughter 1)/(Volume daughter 2). 1.0 gives
  /// equal cells.
  real_t volume_fraction;
  /// phi azimuthal angle (spherical coordinates)
  real_t phi;
  /// theta polar angle (spherical coordinates)
  real_t theta;
};

}  // namespace bdm

#endif  // CORE_AGENT_CELL_DIVISION_EVENT_H_
