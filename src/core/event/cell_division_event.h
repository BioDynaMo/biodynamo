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

#ifndef CORE_EVENT_CELL_DIVISION_EVENT_H_
#define CORE_EVENT_CELL_DIVISION_EVENT_H_

#include "core/event/event.h"

namespace bdm {

/// \brief Contains the parameters to perform a cell division.
///
/// A cell division divides a mother cell in two daughter cells.\n
/// When the mother cell divides, by definition:\n
/// 1) the mother cell becomes the 1st daughter cell\n
/// 2) the new cell becomes the 2nd daughter cell
///
/// The cell that triggers the event is the mother.
struct CellDivisionEvent : public Event {
  static const EventId kEventId;

  CellDivisionEvent(double volume_ratio, double phi, double theta)
      : volume_ratio_(volume_ratio), phi_(phi), theta_(theta) {}

  virtual ~CellDivisionEvent() {}

  EventId GetId() const override { return kEventId; }

  /// volume_ratio_ the ratio (Volume daughter 1)/(Volume daughter 2). 1.0 gives
  /// equal cells.
  double volume_ratio_;
  /// phi_ azimuthal angle (spherical coordinates)
  double phi_;
  /// theta_ polar angle (spherical coordinates)
  double theta_;
};

}  // namespace bdm

#endif  // CORE_EVENT_CELL_DIVISION_EVENT_H_
