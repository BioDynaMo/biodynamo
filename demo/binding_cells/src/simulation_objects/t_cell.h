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

#ifndef T_CELL_H_
#define T_CELL_H_

#include "core/sim_object/cell.h"

namespace bdm {

class Monocyte;

/// Define T-Cell type
class TCell : public Cell {
  BDM_SIM_OBJECT_HEADER(TCell, Cell, 2, is_connected_, connected_cell_, color_);

 public:
  TCell() {}
  explicit TCell(const Double3& position, double diameter, size_t color)
      : Base(position), color_(color) {
    this->SetDiameter(diameter);
  }

  /// Default event constructor
  TCell(const Event& event, SimObject* other, uint64_t new_oid = 0)
      : Base(event, other, new_oid) {}

  void ConnectTo(SoPointer<Monocyte> so) {
    color_ = 2;
    connected_cell_ = so;
  }

  SoPointer<Monocyte> GetConnectedCell() const { return connected_cell_; }

  bool IsConnected() { return connected_cell_ != nullptr; }

  /// Default event handler
  void EventHandler(const Event& event, SimObject* other1,
                    SimObject* other2 = nullptr) override {
    Base::EventHandler(event, other1, other2);
  }

 private:
  // Is this T-Cell connected to a Monocyte
  bool is_connected_ = false;
  // The cell this T-Cell is connected to
  SoPointer<Monocyte> connected_cell_;
  size_t color_ = 1;
};

}  // namespace bdm

#endif  // T_CELL_H_
