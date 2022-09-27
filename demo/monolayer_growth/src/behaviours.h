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

#ifndef BEHAVIOURS_H_
#define BEHAVIOURS_H_

#include "biodynamo.h"
#include "core/behavior/behavior.h"
#include "cycling_cell.h"
#include "sim_param.h"

namespace bdm {

// Define growth behaviour
struct Growth : public Behavior {
  BDM_BEHAVIOR_HEADER(Growth, Behavior, 1);

  Growth() { AlwaysCopyToNew(); }
  virtual ~Growth() {}

  void Run(Agent* agent) override {
    if (auto* cell = dynamic_cast<CyclingCell*>(agent)) {
      auto* random = Simulation::GetActive()->GetRandom();
      real_t ran = random->Uniform(0, 1) * 1.0;
      auto* sparam =
          Simulation::GetActive()
              ->GetParam()
              ->Get<SimParam>();  // get a pointer to an instance of SimParam

      // Counter for Delta t at each stage
      // Used for calculating probability of moving to next state.
      cell->SetDelt(cell->GetDelt() + 0.1);

      // If statements for checking what states we are in and if
      // a cell moves to the next state based on cumulative probability.
      if (cell->GetCycle() == 1) {
        real_t p1 = (cell->GetDelt() / 7) * cell->GetDelt();
        if (p1 > ran) {
          // Changing cells state number or "cycle" postiion.
          cell->SetCycle(2);
          // Delta t is always reset when exiting a state for use in the next
          // state.
          cell->SetDelt(0);
        }

      } else if (cell->GetCycle() == 2) {
        real_t p2 = (cell->GetDelt() / 6) * cell->GetDelt();
        if (p2 > ran) {
          cell->SetCycle(3);
          cell->SetDelt(0);
        }

      } else if (cell->GetCycle() == 3) {
        real_t p3 = (cell->GetDelt() / 3) * cell->GetDelt();
        if (p3 > ran) {
          cell->SetCycle(4);
          cell->SetDelt(0);
        }

      } else {
        real_t p4 = (cell->GetDelt() / 2) * cell->GetDelt();
        if (p4 > ran) {
          cell->SetCycle(1);
          cell->SetDelt(0);
          // Checking if cell has reached the critical volume which leads to
          // cell division. Here 0.975 Vmax is roughly 195% the initial cell
          // volume.
          if (cell->GetVolume() > cell->GetVmax() * 0.975) {
            int neighbours_counter = 0;
            auto* ctxt = Simulation::GetActive()->GetExecutionContext();
            auto check_surrounding_cells =
                L2F([&](Agent* neighbor, real_t squared_distance) {
                  neighbours_counter++;
                });
            ctxt->ForEachNeighbor(check_surrounding_cells, *cell,
                                  (pow(cell->GetDiameter(), 2)));
            // Divide only if the number  of neighbours doesn't exceed a
            // predefined threshold (might represent direct contact signal,
            // pressure, etc)
            if (neighbours_counter <= sparam->neighbours_threshold) {
              cell->Divide(1);
            }
          }
        }
      }

      // Checking if our cells volume is less than the maximum possible valuable
      // achievalbe. if yes cell grows if no then cell does not grow.
      if (cell->GetVolume() < cell->GetVmax()) {
        real_t alpha = 1.0;
        // cell->ChangeVolume(cell->GetVmax() - cell->GetVolume());
        cell->ChangeVolume(
            alpha * cell->GetVolume() *
            ((cell->GetVmax() - cell->GetVolume()) / cell->GetVmax()));
      }

      // cell->SetVrel((cell->GetVolume()/cell->GetVinit())*100.0);
    }
  }
};

}  // namespace bdm

#endif  // BEHAVIOURS_H_