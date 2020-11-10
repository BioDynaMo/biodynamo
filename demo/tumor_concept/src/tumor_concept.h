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
//
// A simulation of a conceptual model of a cancer tumor growth
//

#ifndef DEMO_TUMOR_CONCEPT_H_
#define DEMO_TUMOR_CONCEPT_H_

#include "biodynamo.h"

namespace bdm {

// Define my custom cell MyCell, which extends Cell by adding extra data
// members: cell_color and can_divide
class MyCell : public Cell {  // our object extends the Cell object
                              // create the header with our new data member
  BDM_AGENT_HEADER(MyCell, Cell, 1);

 public:
  MyCell() {}
  explicit MyCell(const Double3& position) : Base(position) {}
  virtual ~MyCell() {}

  /// If MyCell divides, daughter 2 copies the data members from the mother
  void Initialize(const NewAgentEvent& event) override {
    Base::Initialize(event);

    if (auto* mother = dynamic_cast<MyCell*>(event.existing_agent)) {
      cell_color_ = mother->cell_color_;
      if (event.GetUid() == CellDivisionEvent::kUid) {
        // the daughter will be able to divide
        can_divide_ = true;
      } else {
        can_divide_ = mother->can_divide_;
      }
    }
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

// Define growth behaviour
struct Growth : public Behavior {
  BDM_BEHAVIOR_HEADER(Growth, Behavior, 1);

  Growth() { CopyToNewAlways(); }
  virtual ~Growth() {}

  void Run(Agent* agent) override {
    if (auto* cell = dynamic_cast<MyCell*>(agent)) {
      if (cell->GetDiameter() < 8) {
        auto* random = Simulation::GetActive()->GetRandom();
        // Here 400 is the speed and the change to the volume is based on the
        // simulation time step.
        // The default here is 0.01 for timestep, not 1.
        cell->ChangeVolume(400);

        // create an array of 3 random numbers between -2 and 2
        Double3 cell_movements = random->template UniformArray<3>(-2, 2);
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
    param->bound_space = true;
    param->min_bound = 0;
    param->max_bound = 100;  // cube of 100*100*100
  };

  Simulation simulation(argc, argv, set_param);
  auto* rm = simulation.GetResourceManager();
  auto* param = simulation.GetParam();
  auto* myrand = simulation.GetRandom();

  size_t nb_of_cells = 2400;  // number of cells in the simulation
  double x_coord, y_coord, z_coord;

  for (size_t i = 0; i < nb_of_cells; ++i) {
    // our modelling will be a cell cube of 100*100*100
    // random double between 0 and 100
    x_coord = myrand->Uniform(param->min_bound, param->max_bound);
    y_coord = myrand->Uniform(param->min_bound, param->max_bound);
    z_coord = myrand->Uniform(param->min_bound, param->max_bound);

    // creating the cell at position x, y, z
    MyCell* cell = new MyCell({x_coord, y_coord, z_coord});
    // set cell parameters
    cell->SetDiameter(7.5);
    // will vary from 0 to 5. so 6 different layers depending on y_coord
    cell->SetCellColor(static_cast<int>((y_coord / param->max_bound * 6)));

    rm->AddAgent(cell);  // put the created cell in our cells structure
  }

  // create a cancerous cell, containing the behavior Growth
  MyCell* cell = new MyCell({20, 50, 50});
  cell->SetDiameter(6);
  cell->SetCellColor(8);
  cell->SetCanDivide(true);
  cell->AddBehavior(new Growth());
  rm->AddAgent(cell);  // put the created cell in our cells structure

  // Run simulation
  simulation.GetScheduler()->Simulate(500);

  std::cout << "Simulation completed successfully!" << std::endl;
  return 0;
}

}  // namespace bdm

#endif  // DEMO_TUMOR_CONCEPT_H_
