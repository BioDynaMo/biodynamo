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

#ifndef CYCLING_CELL_H_
#define CYCLING_CELL_H_

#include "biodynamo.h"
#include "core/agent/cell.h"

namespace bdm {

// In this example our cell have 4 total states which they can exist in.
enum CellState { kG1, kS, kG2, kM };

class CyclingCell : public Cell {  // our object extends the Cell object
                                   // create the header with our new data member
  BDM_AGENT_HEADER(CyclingCell, Cell, 1);

 public:
  CyclingCell() {}
  explicit CyclingCell(const Real3& position) : Base(position) {}
  virtual ~CyclingCell() {}

  /// If CyclingCell divides, the daughter has to initialize its attributes
  void Initialize(const NewAgentEvent& event) override {
    Base::Initialize(event);

    if (auto* mother = dynamic_cast<CyclingCell*>(event.existing_agent)) {
      cycle_ = mother->cycle_;
      delt_ = mother->delt_;
      vmax_ = mother->vmax_;
      delt_ = mother->vinit_;
      vrel_ = mother->vrel_;
      if (event.GetUid() == CellDivisionEvent::kUid) {
        // the daughter will be able to divide
        can_divide_ = true;
      } else {
        can_divide_ = mother->can_divide_;
      }
    }
  }

  // getter and setter for division
  // A cell can either divide or not divide so is a simple bool.
  void SetCanDivide(bool d) { can_divide_ = d; }
  bool GetCanDivide() const { return can_divide_; }

  // getter and setter for cells current phase of cell cycle
  // keeps track of cells current state
  void SetCycle(CellState c) { cycle_ = c; }
  CellState GetCycle() const { return cycle_; }

  // getter and setter for cells maximum volume
  // in this case cells have a maximum volue they can reach before having to divide
  // thus we set it here.
  void SetVmax(real_t vm) { vmax_ = vm; }
  real_t GetVmax() const { return vmax_; }

  // getter and setter for cells initial volume
  // cells initial volume needs to be handle during simulation so is simply stored
  // here and kept within the agents themselves. Especially important if one wishes
  // to have multiple agents which divide differently/ different initial condtions.
  void SetVinit(real_t vi) { vinit_ = vi; }
  real_t GetVinit() const { return vinit_; }

  // getter and setter for delt t
  // Delt is used for comparing against a cells maximum amount of time it can be within
  // a given state and needs to be kept track of throughout the whole simulation.
  void SetDelt(real_t dt) { delt_ = dt; }
  real_t GetDelt() const { return delt_; }

 private:
  bool can_divide_;
  CellState cycle_ = CellState::kG1;
  real_t vmax_;
  real_t vinit_;
  real_t delt_;
};

}  // namespace bdm

#endif  // CYCLING_CELL_H_
