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
#ifndef CUSTOM_PARAVIEW_H_
#define CUSTOM_PARAVIEW_H_

#include "biodynamo.h"

static std::map<size_t, std::string> dg__ID2Name;
namespace bdm {

inline void cells_4paraview(ResourceManager* rm, int time =0) {
  // iterate to calculate the total number of cells (agents)
  // in the simulation, they will be output as points
  unsigned int n_VTK_points = 0;
  rm->ForEachAgent([&] (Agent* a) {
    if (auto* c = dynamic_cast<Cell*>(a))
      ++n_VTK_points;
  });
  // all cells (agents) will be considered as a cluster of
  // points, denoted as one poly-vertex VTK cell structure
  const unsigned int n_VTK_cells = 1;

  // create the VTU file to store BioDynaMo simulation data
  std::ofstream fout("output/custom_paraview/cells_4paraview."+std::to_string(time)+".vtu");
  // write the header of the XML-structured file
  fout << "<?xml version=\"1.0\"?>" << std::endl;
  fout << "<VTKFile type=\"UnstructuredGrid\" version=\"0.1\" byte_order=\"LittleEndian\">" << std::endl;
  fout << "  <UnstructuredGrid>" << std::endl;
  fout << "    <Piece NumberOfPoints=\"" << n_VTK_points << "\" NumberOfCells=\"" << n_VTK_cells << "\">" << std::endl;
  // output the coordinates of all cells
  fout << "      <Points>" << std::endl;
  fout << "        <DataArray type=\"Float64\" NumberOfComponents=\"3\" format=\"ascii\">" << std::endl;
  rm->ForEachAgent([&] (Agent* a) {
    if (auto* c = dynamic_cast<Cell*>(a))
      fout << ' ' << c->GetPosition()[0]
           << ' ' << c->GetPosition()[1]
           << ' ' << c->GetPosition()[2];
  });
  fout << std::endl;
  fout << "        </DataArray>" << std::endl;
  fout << "      </Points>" << std::endl;
  // start -- output BioDynaMo simulation data for all cells
  fout << "      <PointData>" << std::endl;
  // output the diameter of all cells
  fout << "        <DataArray type=\"Float64\" Name=\"diameter\" NumberOfComponents=\"1\" format=\"ascii\">" << std::endl;
  rm->ForEachAgent([&] (Agent* a) {
    if (auto* c = dynamic_cast<Cell*>(a))
      fout << ' ' << c->GetDiameter();
  });
  fout << std::endl;
  fout << "        </DataArray>" << std::endl;
  // output the volume of all cells
  fout << "        <DataArray type=\"Float64\" Name=\"volume\" NumberOfComponents=\"1\" format=\"ascii\">" << std::endl;
  rm->ForEachAgent([&] (Agent* a) {
    if (auto* c = dynamic_cast<Cell*>(a))
      fout << ' ' << c->GetVolume();
  });
  fout << std::endl;
  fout << "        </DataArray>" << std::endl;
  fout << "      </PointData>" << std::endl;
  // end -- output BioDynaMo simulation data for all cells
  // start -- output all agents as a "VTK_POLY_VERTEX" VTK cell structure
  fout << "      <Cells>" << std::endl;
  fout << "        <DataArray type=\"Int32\" Name=\"offsets\" NumberOfComponents=\"1\" format=\"ascii\">" << std::endl;
  fout << ' ' << n_VTK_points << std::endl;
  fout << "        </DataArray>" << std::endl;
  fout << "        <DataArray type=\"Int32\" Name=\"connectivity\" NumberOfComponents=\"1\" format=\"ascii\">" << std::endl;
  for (unsigned int p=0; p<n_VTK_points; p++) fout << ' ' << p;
  fout << std::endl;
  fout << "        </DataArray>" << std::endl;
  fout << "        <DataArray type=\"Int32\" Name=\"types\" NumberOfComponents=\"1\" format=\"ascii\">" << std::endl;
  fout << ' ' << 2 << std::endl;
  fout << "        </DataArray>" << std::endl;
  fout << "      </Cells>" << std::endl;
  // end -- output all agents as a "VTK_POLY_VERTEX" VTK cell structure
  fout << "    </Piece>" << std::endl;
  fout << "  </UnstructuredGrid>" << std::endl;
  fout << "</VTKFile>" << std::endl;
  // completed exporting to the VTU file
}

inline void diffusiongrid_4paraview(ResourceManager* rm, int time =0) {

}

inline int Simulate(int argc, const char** argv) {
  Simulation simulation(argc, argv);

  // access the simulation resource manager
  auto* rm = simulation.GetResourceManager();

  // create 8 cells (agents) positioned at the
  // vertices of a perfect hexagon
  rm->AddAgent(new Cell({-1.0, -1.0, -1.0}));
  rm->AddAgent(new Cell({+1.0, -1.0, -1.0}));
  rm->AddAgent(new Cell({+1.0, +1.0, -1.0}));
  rm->AddAgent(new Cell({-1.0, +1.0, -1.0}));
  rm->AddAgent(new Cell({-1.0, -1.0, +1.0}));
  rm->AddAgent(new Cell({+1.0, -1.0, +1.0}));
  rm->AddAgent(new Cell({+1.0, +1.0, +1.0}));
  rm->AddAgent(new Cell({-1.0, +1.0, +1.0}));

  // output all agent-data in a VTU-formatted
  // file for Paraview
  cells_4paraview(rm);

  // create 2 biochemical cues
  dg__ID2Name[0] = "O2";
  dg__ID2Name[1] = "GF";
  ModelInitializer::DefineSubstance(0, dg__ID2Name[0], 0.0, 0.0);
  ModelInitializer::DefineSubstance(1, dg__ID2Name[1], 0.0, 0.0);

  // output all diffusion grid-data in a VTU-formatted
  // file for Paraview
  diffusiongrid_4paraview(rm);

  std::cout << "Simulation completed successfully!" << std::endl;
  return 0;
}

}  // namespace bdm

#endif  // CUSTOM_PARAVIEW_H_
