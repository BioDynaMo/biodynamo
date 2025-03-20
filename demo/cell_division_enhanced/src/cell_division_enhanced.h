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
//
// \title Cell Division Enhanced
// \visualize
//
// This model creates a grid of 4x4x4 cells. Each cell grows until a specific
// volume, after which it divides into two equally-sized cells.
//

#ifndef DEMO_CELL_DIVISION_ENHANCED_H_
#define DEMO_CELL_DIVISION_ENHANCED_H_

#include <random>
#include "biodynamo.h"

template <class T>
inline int signum(const T& x) {
  return (x >= 0 ? 1 : -1);
}

namespace bdm {

class MyCell : public Cell {
  BDM_AGENT_HEADER(MyCell, Cell, 1);

 public:
  MyCell() : Cell() { UpdateVolume(); }

  explicit MyCell(real_t diameter) : Cell(diameter) { UpdateVolume(); }

  explicit MyCell(const Real3& position) : Cell(position) { UpdateVolume(); }

  ~MyCell() override = default;

  /// \brief This method is used to initialise the values of daughter
  /// 2 for a cell division event.
  ///
  /// \see CellDivisionEvent
  void Initialize(const NewAgentEvent& event) override {
    Base::Initialize(event);

    if (event.GetUid() == CellDivisionEvent::kUid) {
      auto* mother = bdm_static_cast<MyCell*>(event.existing_agent);
      auto* daughter = this;  // FIXME

      std::random_device rd;
      std::mt19937 gen(rd());
      std::uniform_int_distribution<> distro(-1, 1);

      real_t radius = mother->GetDiameter() * real_t(0.5);
      real_t theta = bdm::Math::ToRadian(distro(gen) * 45.0);
      real_t phi = bdm::Math::ToRadian(distro(gen) * 45.0);

      // define an axis for division (along which the nuclei will move)
      real_t x_coord = std::cos(theta) * std::sin(phi);
      real_t y_coord = std::sin(theta) * std::sin(phi);
      real_t z_coord = std::cos(phi);
      Real3 coords = {x_coord, y_coord, z_coord};
      real_t total_length_of_displacement = radius / real_t(1.5);

      Real3 axis_of_division = total_length_of_displacement *
                               (coords.EntryWiseProduct(mother->kXAxis) +
                                coords.EntryWiseProduct(mother->kYAxis) +
                                coords.EntryWiseProduct(mother->kZAxis));

      real_t d_ =
          real_t(0.5) * total_length_of_displacement * signum(distro(gen));

      // set position of mother and daughter cell
      auto xyz = mother->GetPosition();
      daughter->SetPosition(xyz + (d_ * axis_of_division));
      mother->SetPosition(xyz - (d_ * axis_of_division));

      // copy properties from mother to daughter cell
      daughter->SetAdherence(mother->GetAdherence());
      daughter->SetDensity(mother->GetDensity());

      // update volume of mother and daughter cell
      real_t mother_volume = mother->GetVolume();
      daughter->SetVolume(real_t(0.5) * mother_volume);
      mother->SetVolume(real_t(0.5) * mother_volume);
    }
  }
};

namespace cell_division_enhanced {

inline int Simulate(int argc, const char** argv) {
  // Create a new simulation
  Simulation simulation(argc, argv);

  // Let's define the number of cells we wish to create along each dimension,
  // the spacing between the cells, and each cell's diameter.
  size_t cells_per_dim = 4;
  size_t spacing = 20;

  // To define how are cells will look like we will create a construct in the
  // form of a C++ lambda as follows.
  auto construct = [&](const Real3& position) {
    size_t diameter_min = 5;
    size_t diameter_max = 10;
    real_t growth_ratio = 50;

    auto cell = new MyCell(position);
    cell->SetDiameter(diameter_min);
    // Add the "grow and divide" behavior to each cell
    cell->AddBehavior(new GrowthDivision(diameter_max, growth_ratio));
    return cell;
  };
  ModelInitializer::Grid3D(cells_per_dim, spacing, construct);

  // Run simulation for a few time-steps
  simulation.GetScheduler()->Simulate(111);

  std::cout << "Simulation completed successfully!" << std::endl;
  return 0;
}

}  // namespace cell_division_enhanced
}  // namespace bdm

#endif  // DEMO_CELL_DIVISION_ENHANCED_H_
