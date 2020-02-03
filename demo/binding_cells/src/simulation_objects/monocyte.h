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

#ifndef MONOCYTE_H_
#define MONOCYTE_H_

#include "core/sim_object/cell.h"
#include "simulation_objects/t_cell.h"

namespace bdm {

/// Define Monocyte cell type
class Monocyte : public Cell {
  BDM_SIM_OBJECT_HEADER(Monocyte, Cell, 2, connected_cells_, max_connections_,
                        at_bottom_, color_, inhibited_);

 public:
  Monocyte() {}
  explicit Monocyte(const Double3& position, double diameter, size_t color)
      : Base(position), color_(color) {
    this->SetDiameter(diameter);
  }

  /// Default event constructor
  Monocyte(const Event& event, SimObject* other, uint64_t new_oid = 0)
      : Base(event, other, new_oid) {}

  void ConnectTo(SoPointer<TCell> so) {
    if (connected_cells_.empty()) {
      color_ = 2;
    }
    connected_cells_.push_back(so);
  }

  std::vector<SoPointer<TCell>> GetConnectedCells() const {
    return connected_cells_;
  }

  void SetMaximumNumberOfSynapses(size_t num) { max_connections_ = num; }

  void StickToWellBottom() { at_bottom_ = true; }

  bool AtBottom() { return at_bottom_; }

  bool IsInhibited() const { return inhibited_; }

  void Inhibit() { inhibited_ = true; }

  bool IsOccupied() const {
    return connected_cells_.size() == max_connections_;
  }

  /// Default event handler
  void EventHandler(const Event& event, SimObject* other1,
                    SimObject* other2 = nullptr) override {
    Base::EventHandler(event, other1, other2);
  }

 private:
  // The cells that are connected to this Monocyte
  std::vector<SoPointer<TCell>> connected_cells_;
  // The maximum number of connections allowed to this Monocyte
  size_t max_connections_ = 1;
  // Is the monocyte at the bottom of the cell well?
  bool at_bottom_ = false;
  size_t color_ = 0;
  // Is this T-Cell inhibited (i.e. due to PD-1 <-> PD-L1 interaction)
  bool inhibited_ = false;
};

}  // namespace bdm

#endif  // MONOCYTE_H_
