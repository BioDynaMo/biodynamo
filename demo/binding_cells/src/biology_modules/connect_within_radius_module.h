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
#include "biology_modules/constant_displacement_module.h"
#include "simulation_objects/my_cell.h"

namespace bdm {

struct ConnectWithinRadius : public BaseBiologyModule {
 public:
  ConnectWithinRadius() : BaseBiologyModule(gAllEventIds) {}

  ConnectWithinRadius(double radius = 1, size_t pct = 0)
      : BaseBiologyModule(gAllEventIds),
        squared_radius_(radius * radius),
        preferred_cell_type_(pct) {}

  ConnectWithinRadius(const Event& event, BaseBiologyModule* other,
                      uint64_t new_oid = 0)
      : BaseBiologyModule(event, other, new_oid) {
    if (ConnectWithinRadius* gdbm = dynamic_cast<ConnectWithinRadius*>(other)) {
      squared_radius_ = gdbm->squared_radius_;
      preferred_cell_type_ = gdbm->preferred_cell_type_;
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

  template <typename ACell>
  void Connect(ACell cell_a, ACell cell_b) {
    Double3 distance = cell_b->GetPosition() - cell_a->GetPosition();
    auto radius_a = cell_a->GetDiameter() / 2;
    Double3 displacement = distance - radius_a;
    cell_a->UpdatePosition(displacement);
  }

  void Run(SimObject* so) override {
    if (auto* this_cell = dynamic_cast<MyCell*>(so)) {
      // Prune if we are already connected to some other cell
      if (this_cell->IsConnected()) {
        return;
      }

      SoPointer<MyCell> cell_to_connect_to;  // nullptr initially
      double smallest_distance = Math::kInfinity;
      auto find_closest_cell = [&](const auto* neighbor) {
        if (auto* neighbor_cell = dynamic_cast<const MyCell*>(neighbor)) {
          if (at_most_one_connection_ && !neighbor_cell->IsOccupied() &&
              !neighbor_cell->IsInhibited()) {
            if (neighbor_cell->GetCellType() == preferred_cell_type_) {
              double distance = SquaredEuclideanDistance(
                  neighbor_cell->GetPosition(), this_cell->GetPosition());
              if (distance < smallest_distance) {
                smallest_distance = distance;
                cell_to_connect_to = SoPointer<MyCell>(neighbor_cell->GetUid());
              }
            }
          }
        } else {
          Log::Fatal(
              "ConnectWithinRadius::ConnectToNearestCell",
              "found a cell"
              " that is within radius of interest, but is not of type MyCell!");
        }
      };
      auto* ctxt = Simulation::GetActive()->GetExecutionContext();
      ctxt->ForEachNeighborWithinRadius(find_closest_cell, *this_cell,
                                        squared_radius_);
      auto t = Simulation::GetActive()->GetScheduler()->GetSimulatedSteps();
      if (cell_to_connect_to != nullptr && (t != 0)) {
        this_cell->ConnectTo(cell_to_connect_to);
        cell_to_connect_to->MakeOccupied();
      }
    } else {
      Log::Fatal("ConnectWithinRadius::ConnectToNearestCell",
                 "running this BM"
                 " on a simulation object that is not of type MyCell!");
    }
  }

 private:
  double squared_radius_;
  size_t preferred_cell_type_;
  bool at_most_one_connection_ = true;
};

}  // namespace bdm

#endif  // CONNECT_WITHIN_RADIUS_MODULE_H_
