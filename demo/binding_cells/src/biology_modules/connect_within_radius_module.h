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
#ifndef CONNECT_WITHIN_RADIUS_MODULE_H_
#define CONNECT_WITHIN_RADIUS_MODULE_H_

#include "biodynamo.h"
#include "core/environment/uniform_grid_environment.h"
#include "biology_modules/constant_displacement_module.h"
#include "simulation_objects/monocyte.h"
#include "simulation_objects/t_cell.h"

namespace bdm {

struct ConnectWithinRadius : public BaseBiologyModule {
 public:
  ConnectWithinRadius() : BaseBiologyModule(gAllEventIds) {}

  ConnectWithinRadius(double radius = 1)
      : BaseBiologyModule(gAllEventIds), squared_radius_(radius * radius) {}

  ConnectWithinRadius(const Event& event, BaseBiologyModule* other,
                      uint64_t new_oid = 0)
      : BaseBiologyModule(event, other, new_oid) {
    if (ConnectWithinRadius* gdbm = dynamic_cast<ConnectWithinRadius*>(other)) {
      squared_radius_ = gdbm->squared_radius_;
    } else {
      Log::Fatal("ConnectWithinRadius::EventConstructor",
                 "other was not of type ConnectWithinRadius");
    }
  }

  /// Create a new instance of this object using the default constructor.
  BaseBiologyModule* GetInstance(const Event& event, BaseBiologyModule* other,
                                 uint64_t new_oid = 0) const override {
    return new ConnectWithinRadius(event, other, new_oid);
  }

  /// Create a copy of this biology module.
  BaseBiologyModule* GetCopy() const override {
    return new ConnectWithinRadius(*this);
  }

  void Run(SimObject* so) override {
    if (auto* this_cell = static_cast<TCell*>(so)) {
      // Prune if we are already connected to a monocyte
      if (this_cell->IsConnected()) {
        return;
      }

      SoPointer<Monocyte> cell_to_connect_to;  // nullptr initially
      double smallest_distance = Math::kInfinity;
      auto find_closest_cell = [&](const SimObject* neighbor) {
        if (auto* neighbor_cell = dynamic_cast<const Monocyte*>(neighbor)) {
          // T-Cells are activated if they are in close vicinity of monocytes
          if (!this_cell->IsActivated() || !this_cell->IsConnected()) {
            this_cell->Activate();
          }

          // We can only form an immune synapse with a monocyte if there is
          // physically enough room and if the pathway is not inhibited
          if (!neighbor_cell->IsOccupied() && !neighbor_cell->IsInhibited()) {
            double distance = SquaredEuclideanDistance(
                neighbor_cell->GetPosition(), this_cell->GetPosition());
            if (distance < smallest_distance) {
              smallest_distance = distance;
              cell_to_connect_to = SoPointer<Monocyte>(neighbor_cell->GetUid());
            }
          }
        }
      };
      auto* grid = static_cast<UniformGridEnvironment*>(
          Simulation::GetActive()->GetEnvironment());
      grid->ForEachNeighborWithinRadius(find_closest_cell, *this_cell,
                                        squared_radius_);

      // If we found an available monocyte then we connect this cell and the
      // monocyte with each other, forming an immune synapse
      if (cell_to_connect_to != nullptr) {
        this_cell->ConnectTo(cell_to_connect_to);
        SoPointer<TCell> soptr(this_cell->GetUid());
        cell_to_connect_to->ConnectTo(soptr);
        // A T-Cell is considered not activated when it formed an immune synapse
        this_cell->Deactivate();
      }
    }
  }

 private:
  double squared_radius_;
};

}  // namespace bdm

#endif  // CONNECT_WITHIN_RADIUS_MODULE_H_
