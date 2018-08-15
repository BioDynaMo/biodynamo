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

#ifndef CANCER_GROWTH_H_
#define CANCER_GROWTH_H_

#include "biodynamo.h"

namespace bdm {

// -----------------------------------------------------------------------------
// This model creates single cancerous cell. This cell divides recursively.
// Cells that have many cells in their neighbourhood, divide slower, because of 
// lack of nutrients. When the cell has certain numbers of neighborhood cells, 
// it stops growing -- its growth_rate becomes 0 -- and changes its own 
// cell_color.  
// -----------------------------------------------------------------------------

// 0. Define my custom cell MyCell, which extends Cell by adding extra data
// member cell_color
BDM_SIM_OBJECT(MyCell, Cell) {  // our object extends the Cell object
                                // create the header with our new data member
  BDM_SIM_OBJECT_HEADER(MyCellExt, 1, cell_color_);

 public:
  MyCellExt() {}
  explicit MyCellExt(const std::array<double, 3>& position) : Base(position) {}

  // getter and setter for our new data member
  void SetCellColor(int cell_color) { cell_color_[kIdx] = cell_color; }
  int GetCellColor() const { return cell_color_[kIdx]; }

 private:
  // declare new data member and define their type
  // private data can only be accessed by public function and not directly
  vec<int> cell_color_;
};

// 1. Define growth behavior
struct GrowthModule : public BaseBiologyModule {
  GrowthModule() : BaseBiologyModule(gAllBmEvents) {}
  GrowthModule(double threshold, double growth_rate, double neighborhood_radius, 
						int maximum_of_neighbors) : 
				BaseBiologyModule(gAllBmEvents), 
				threshold_(threshold), 
				growth_rate_(growth_rate),
				neighborhood_radius_(neighborhood_radius),
				maximum_of_neighbors_(maximum_of_neighbors) {}

  template <typename T, typename TSimulation = Simulation<>>
  void Run(T* cell) {
    auto* grid = TSimulation::GetActive()->GetGrid();
    // use atomic counter to get rid of race condition
    std::atomic<int> neighbors{0};
    auto increment_neighbors = [&](auto&& lhs, SoHandle lhs_id){
      neighbors++;
    };
    grid->ForEachNeighborWithinRadius(increment_neighbors, *cell, 
	cell->GetSoHandle(), (threshold_) * (threshold_));
    // if cell is bigger than or equal to growth threshold, then divide
    if (cell->GetDiameter() >= threshold_) {
      cell->Divide();
    }   
    // if cell is less than growth threshold 
    else {
      if (neighbors != 0) {
        if (neighbors < maximum_of_neighbors_) {
  	  // volume of cell's growth is inversely proportional
          cell->ChangeVolume(growth_rate_ / neighbors);
        }
        else {
          // if there is more than maximum neigbors, cell would stop growing
          // and repaint (in red)
          cell->ChangeVolume(0);
          cell->SetCellColor(cell->GetCellColor() + neighbors);
        }
      }      
    }
  }

 private:
  // default values
  double threshold_ = 30;
  double growth_rate_ = 1500;
  double neighborhood_radius_ = 30;
  int maximum_of_neighbors_ = 5;

  ClassDefNV(GrowthModule, 1);
};

// Define compile time parameter
template <typename Backend>
struct CompileTimeParam : public DefaultCompileTimeParam<Backend> {
  using BiologyModules = Variant<GrowthModule>; // add GrowthModule 
  using AtomicTypes = VariadicTypedef<MyCell>; // use MyCell object
};

inline int Simulate(int argc, const char** argv) {
  Simulation<> simulation(argc, argv); 
  
  // 2. Define initial model, which is consisting of one cell at the beginning
  size_t cells_per_dim = 1;
  auto construct = [](const std::array<double, 3>& position) {
    MyCell cell(position);
    cell.SetDiameter(30);
    cell.SetAdherence(0.4);
    cell.SetMass(1.0);
    cell.SetCellColor(8);
    // add GrowthModule 
    cell.AddBiologyModule(GrowthModule()); // here by adding argument values inside
					   // GrowthModule(_) user can set variables
					   // of simulation    
    return cell;
  };
  ModelInitializer::Grid3D(cells_per_dim, 1, construct);

  // Run simulation for N timestep
  simulation.GetScheduler()->Simulate(10000);

  std::cout << "Simulation completed successfully!" << std::endl;
  return 0;
}

}  // namespace bdm


#endif  // CANCER_GROWTH_H_
