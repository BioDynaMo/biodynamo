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

#ifndef MY_CELL_H_
#define MY_CELL_H_

#include <biodynamo.h>

namespace bdm {

/// Define my custom cell, which extends Cell by adding an extradata member
/// cell_type
class MyCell : public Cell {
  BDM_SIM_OBJECT_HEADER(MyCell, Cell, 2, connected_cell_, occupied_, inhibited_, cell_type_);

 public:
  MyCell() {}
  explicit MyCell(const Double3& position, double diameter, size_t cell_type) : Base(position), cell_type_(cell_type) {
    this->SetDiameter(diameter);
  }

  /// Default event constructor
  MyCell(const Event& event, SimObject* other, uint64_t new_oid = 0)
      : Base(event, other, new_oid) {}

  void ConnectTo(SoPointer<MyCell> so) {
    auto t = Simulation::GetActive()->GetScheduler()->GetSimulatedSteps();
    if (t == 0)
      std::cout << "what the heck 0" << std::endl;
    connected_cell_ = so; 
  }

  void Disconnect() { connected_cell_ = nullptr; }

  SoPointer<MyCell> GetConnectedCell() const { return connected_cell_; }

  bool IsConnected() { return connected_cell_ != nullptr; }

  bool IsOccupied() const { return occupied_; }

  bool IsInhibited() const { return inhibited_; }

  void Inhibit() { inhibited_ = true; }

  void MakeOccupied() { occupied_ = true; }

  /// Default event handler
  void EventHandler(const Event& event, SimObject* other1,
                    SimObject* other2 = nullptr) override {
    Base::EventHandler(event, other1, other2);
  }

  size_t GetCellType() const { return cell_type_; }

 private:
  SoPointer<MyCell> connected_cell_;
  bool occupied_ = false;
  bool inhibited_ = false;
  size_t cell_type_;
};

}  // namespace bdm

#endif  // MY_CELL_H_
