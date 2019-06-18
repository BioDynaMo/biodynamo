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

#ifndef TUMOR_CONCEPT_H_
#define TUMOR_CONCEPT_H_

#include "biodynamo.h"

namespace bdm {

// 0. Define my custom cell MyCell, which extends Cell by adding extra data
// members: cell_color and can_divide
class MyCell : public Cell {  // our object extends the Cell object
                              // create the header with our new data member
  BDM_SIM_OBJECT_HEADER(MyCell, Cell, 1, can_divide_, cell_color_);

 public:
  MyCell() {}
  explicit MyCell(const std::array<double, 3>& position) : Base(position) {}

  /// If MyCell divides, daughter 2 copies the data members from the mother
  MyCell(const Event& event, SimObject* other, uint64_t new_oid = 0)
      : Base(event, other, new_oid) {
    if (auto* mother = dynamic_cast<MyCell*>(other)) {
      cell_color_ = mother->cell_color_;
      if (event.GetId() == CellDivisionEvent::kEventId) {
        // the daughter will be able to divide
        can_divide_ = true;
      } else {
        can_divide_ = mother->can_divide_;
      }
    }
  }

  /// If a cell divides, daughter keeps the same state from its mother.
  void EventHandler(const Event& event, SimObject* other1,
                    SimObject* other2 = nullptr) override {
    Base::EventHandler(event, other1, other2);
  }

  // getter and setter for our new data member
  void SetCanDivide(bool d) { can_divide_ = d; }
  bool GetCanDivide() const { return can_divide_; }

  void SetCellColor(int cell_color) { cell_color_ = cell_color; }
  int GetCellColor() const { return cell_color_; }

 private:
  // declare new data member and define their type
  // private data can only be accessed by public function and not directly
  bool can_divide_;
  int cell_color_;
};

// 1. Define growth behaviour
struct GrowthModule : public BaseBiologyModule {
  BDM_STATELESS_BM_HEADER(GrowthModule, BaseBiologyModule, 1);

 public:
  GrowthModule() : BaseBiologyModule(gAllEventIds) {}

  /// Empty default event constructor, because GrowthModule does not have state.
  template <typename TEvent, typename TBm>
  GrowthModule(const TEvent& event, TBm* other, uint64_t new_oid = 0)
      : BaseBiologyModule(event, other, new_oid) {}

  /// event handler not needed, because Chemotaxis does not have state.

  void Run(SimObject* so) override {
    if (auto* cell = dynamic_cast<MyCell*>(so)) {
      if (cell->GetDiameter() < 8) {
        auto* random = Simulation::GetActive()->GetRandom();
        cell->ChangeVolume(400);

        // create an array of 3 random numbers between -2 and 2
        std::array<double, 3> cell_movements =
            random->template UniformArray<3>(-2, 2);
        // update the cell mass location, ie move the cell
        cell->UpdatePosition(cell_movements);
      } else {  //
        auto* random = Simulation::GetActive()->GetRandom();

        if (cell->GetCanDivide() && random->Uniform(0, 1) < 0.8) {
          cell->Divide();
        } else {
          cell->SetCanDivide(false);  // this cell won't divide anymore
        }
      }
    }
  }
};

inline int Simulate(int argc, const char** argv) {
  auto set_param = [](Param* param) {
    param->bound_space_ = true;
    param->min_bound_ = 0;
    param->max_bound_ = 100;  // cube of 100*100*100
  };

  Simulation simulation(argc, argv, set_param);
  auto* rm = simulation.GetResourceManager();
  auto* param = simulation.GetParam();
  auto* random = simulation.GetRandom();
  // Since sim_objects in this simulation won't modify neighbors, we can
  // safely disable neighbor guards to improve performance.
  simulation.GetExecutionContext()->DisableNeighborGuard();

  size_t nb_of_cells = 2400;  // number of cells in the simulation
  double x_coord, y_coord, z_coord;

  for (size_t i = 0; i < nb_of_cells; ++i) {
    // our modelling will be a cell cube of 100*100*100
    // random double between 0 and 100
    x_coord = random->Uniform(param->min_bound_, param->max_bound_);
    y_coord = random->Uniform(param->min_bound_, param->max_bound_);
    z_coord = random->Uniform(param->min_bound_, param->max_bound_);

    // creating the cell at position x, y, z
    MyCell* cell = new MyCell({x_coord, y_coord, z_coord});
    // set cell parameters
    cell->SetDiameter(7.5);
    // will vary from 0 to 5. so 6 different layers depending on y_coord
    cell->SetCellColor(static_cast<int>((y_coord / param->max_bound_ * 6)));

    rm->push_back(cell);  // put the created cell in our cells structure
  }

  // create a cancerous cell, containing the biology module GrowthModule
  MyCell* cell = new MyCell({20, 50, 50});
  cell->SetDiameter(6);
  cell->SetCellColor(8);
  cell->SetCanDivide(true);
  cell->AddBiologyModule(new GrowthModule());
  rm->push_back(cell);  // put the created cell in our cells structure

  // Run simulation
  simulation.GetScheduler()->Simulate(500);

  std::cout << "Simulation completed successfully!" << std::endl;
  return 0;
}

}  // namespace bdm

#endif  // TUMOR_CONCEPT_H_
