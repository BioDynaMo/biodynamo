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

#include "core/exporter.h"

#include <array>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>

#include "core/param/param.h"
#include "core/resource_manager.h"
#include "core/sim_object/cell.h"
#include "core/simulation.h"
#include "core/util/log.h"

namespace bdm {

Exporter::~Exporter() {}

void BasicExporter::ExportIteration(std::string filename, uint64_t iteration) {
  std::ofstream outfile;
  outfile.open(filename);
  auto* rm = Simulation::GetActive()->GetResourceManager();
  rm->ApplyOnAllElements([&](SimObject* so) {
    auto curr_pos = so->GetPosition();
    outfile << "[" << curr_pos[0] << "," << curr_pos[1] << "," << curr_pos[2]
            << "]" << std::endl;
  });

  outfile.close();
}

void BasicExporter::ExportSummary(std::string filename,
                                  uint64_t num_iterations) {}

// -----------------------------------------------------------------------------
void MatlabExporter::ExportIteration(std::string filename, uint64_t iteration) {
  std::ofstream outfile;
  outfile.open(filename);

  auto* rm = Simulation::GetActive()->GetResourceManager();
  auto num_cells = rm->GetNumSimObjects();
  outfile << "CellPos = zeros(" << num_cells << "," << 3 << ");" << std::endl;

  uint64_t i = 0;
  rm->ApplyOnAllElements([&](SimObject* so) {
    auto curr_pos = so->GetPosition();
    outfile << "CellPos(" << i++ + 1 << ",1:3) = [" << curr_pos[0] << ","
            << curr_pos[1] << "," << curr_pos[2] << "];" << std::endl;
  });

  outfile.close();
}

void MatlabExporter::ExportSummary(std::string filename,
                                   uint64_t num_iterations) {}

// -----------------------------------------------------------------------------
void NeuroMLExporter::ExportIteration(std::string filename,
                                      uint64_t iteration) {
  std::ofstream outfile;
  outfile.open(filename);

  std::string space1 = "   ";
  std::string space2 = "      ";
  std::string space3 = "         ";
  std::string space4 = "            ";

  outfile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
  outfile << "<neuroml xmlns      = \"http://morphml.org/neuroml/schema\""
          << std::endl;
  outfile << space1
          << "xmlns:xsi  = \"http://www.w3.org/2001/XMLSchema-instance\" "
          << std::endl;
  outfile << space1 << "xmlns:meta = \"http://morphml.org/metadata/schema\" "
          << std::endl;
  outfile << space1 << "xsi:schemaLocation=\"http://morphml.org/neuroml/"
                       "schema NeuroML_Level3_v1.7.1.xsd\" "
          << std::endl;
  outfile << space1 << "lengthUnits=\"micrometer\" " << std::endl;

  outfile << space1 << "<cells>" << std::endl;

  /// In the future, the electrophysiological properties of neurons can be
  /// inserted instead of these prespecified values
  outfile << space2 << "<cell name=\"exz_lif\"> " << std::endl;
  outfile << space3 << "<meta:properties>" << std::endl;
  outfile << space4 << "<meta:property tag=\"PointNeuronModel\" value=\"yes\"/>"
          << std::endl;
  outfile << space4 << "<meta:property tag=\"Class\"    value=\"CbLifNeuron\"/>"
          << std::endl;
  outfile << space4 << "<meta:property tag=\"Cm\"       value=\"5e-10\"/>"
          << std::endl;
  outfile << space4 << "<meta:property tag=\"Rm\"       value=\"1e8\"/>"
          << std::endl;
  outfile << space4 << "<meta:property tag=\"Vthresh\"  value=\"-50e-3\"/>"
          << std::endl;
  outfile << space4 << "<meta:property tag=\"Vresting\" value=\"-60e-3\"/>"
          << std::endl;
  outfile << space4 << "<meta:property tag=\"Vreset\"   value=\"-60e-3\"/>"
          << std::endl;
  outfile << space4 << "<meta:property tag=\"Trefract\" value=\"5e-3\"/>"
          << std::endl;
  outfile << space4 << "<meta:property tag=\"Vinit\"    value=\"-60e-3\"/>"
          << std::endl;
  outfile << space3 << "</meta:properties>" << std::endl;
  outfile << space2 << "</cell>" << std::endl;

  outfile << space2 << "<cell name=\"inh_lif\"> " << std::endl;
  outfile << space3 << "<meta:properties>" << std::endl;
  outfile << space4 << "<meta:property tag=\"PointNeuronModel\" value=\"yes\"/>"
          << std::endl;
  outfile << space4 << "<meta:property tag=\"Class\"    value=\"CbLifNeuron\"/>"
          << std::endl;
  outfile << space4 << "<meta:property tag=\"Cm\"       value=\"2e-10\"/>"
          << std::endl;
  outfile << space4 << "<meta:property tag=\"Rm\"       value=\"1e8\"/>"
          << std::endl;
  outfile << space4 << "<meta:property tag=\"Vthresh\"  value=\"-50e-3\"/>"
          << std::endl;
  outfile << space4 << "<meta:property tag=\"Vresting\" value=\"-60e-3\"/>"
          << std::endl;
  outfile << space4 << "<meta:property tag=\"Vreset\"   value=\"-60e-3\"/>"
          << std::endl;
  outfile << space4 << "<meta:property tag=\"Trefract\" value=\"5e-3\"/>"
          << std::endl;
  outfile << space4 << "<meta:property tag=\"Vinit\"    value=\"-60e-3\"/>"
          << std::endl;
  outfile << space3 << "</meta:properties>" << std::endl;
  outfile << space2 << "</cell>" << std::endl;
  outfile << space1 << "</cells>" << std::endl;

  /// TODO(roman): here, the cell populations and connectivity will be
  /// specified and exported, once these are included in the model
  // auto* rm = Simulation::GetActive()->GetResourceManager();
  // rm->ApplyOnAllElements([&](SimObject* so) { });

  outfile << std::endl;
  outfile << "</neuroml>" << std::endl;

  outfile.close();

  Log::Info("Exporter", "Created NeuroML file");
}

void NeuroMLExporter::ExportSummary(std::string filename,
                                    uint64_t num_iterations) {}

// -----------------------------------------------------------------------------
void ParaviewExporter::ExportIteration(std::string filename,
                                       uint64_t iteration) {
  auto* rm = Simulation::GetActive()->GetResourceManager();
  auto num_cells = rm->GetNumSimObjects();
  size_t index = 0;
  std::ofstream vtu(filename + "-" + std::to_string(iteration) + ".vtu");

  vtu << "<VTKFile type=\"UnstructuredGrid\" version=\"0.1\" "
         "byte_order=\"LittleEndian\">"
      << std::endl;
  vtu << "   <UnstructuredGrid>" << std::endl;
  vtu << "      <Piece  NumberOfPoints=\"" << num_cells << "\" NumberOfCells=\""
      << num_cells << "\">" << std::endl;
  vtu << "         <Points>" << std::endl;
  vtu << "            <DataArray type=\"Float64\" NumberOfComponents=\"3\" "
         "format=\"ascii\">"
      << std::endl;
  rm->ApplyOnAllElements([&](SimObject* so) {
    auto& coord = so->GetPosition();
    vtu << ' ' << coord[0] << ' ' << coord[1] << ' ' << coord[2] << std::flush;
  });
  vtu << std::endl;
  vtu << "            </DataArray>" << std::endl;
  vtu << "         </Points>" << std::endl;
  vtu << "         <PointData>" << std::endl;
  vtu << "            <DataArray type=\"Float64\" Name=\"Cell_ID\" "
         "NumberOfComponents=\"1\" format=\"ascii\">"
      << std::endl;
  index = 0;
  for (size_t i = 0; i < num_cells; i++) {
    vtu << ' ' << index++ << std::flush;
  }
  vtu << std::endl;
  vtu << "            </DataArray>" << std::endl;
  vtu << "            <DataArray type=\"Float64\" Name=\"Adherence\" "
         "NumberOfComponents=\"1\" format=\"ascii\">"
      << std::endl;

  rm->ApplyOnAllElements([&](SimObject* so) {
    if (auto* cell = dynamic_cast<Cell*>(so)) {
      auto adhr = cell->GetAdherence();
      vtu << ' ' << adhr << std::flush;
    }
  });

  vtu << std::endl;
  vtu << "            </DataArray>" << std::endl;
  vtu << "            <DataArray type=\"Float64\" Name=\"Diameter\" "
         "NumberOfComponents=\"1\" format=\"ascii\">"
      << std::endl;
  rm->ApplyOnAllElements([&](SimObject* so) {
    auto diam = so->GetDiameter();
    vtu << ' ' << diam << std::flush;
  });

  vtu << std::endl;
  vtu << "            </DataArray>" << std::endl;
  vtu << "            <DataArray type=\"Float64\" Name=\"Mass\" "
         "NumberOfComponents=\"1\" format=\"ascii\">"
      << std::endl;

  rm->ApplyOnAllElements([&](SimObject* so) {
    if (auto* cell = dynamic_cast<Cell*>(so)) {
      auto mass = cell->GetMass();
      vtu << ' ' << mass << std::flush;
    }
  });

  vtu << std::endl;
  vtu << "            </DataArray>" << std::endl;
  vtu << "            <DataArray type=\"Float64\" Name=\"Volume\" "
         "NumberOfComponents=\"1\" format=\"ascii\">"
      << std::endl;

  rm->ApplyOnAllElements([&](SimObject* so) {
    if (auto* cell = dynamic_cast<Cell*>(so)) {
      auto v = cell->GetVolume();
      vtu << ' ' << v << std::flush;
    }
  });

  vtu << std::endl;
  vtu << "            </DataArray>" << std::endl;
  vtu << "            <DataArray type=\"Float64\" Name=\"TractionForce\" "
         "NumberOfComponents=\"3\" format=\"ascii\">"
      << std::endl;

  rm->ApplyOnAllElements([&](SimObject* so) {
    if (auto* cell = dynamic_cast<Cell*>(so)) {
      auto& tracf = cell->GetTractorForce();
      vtu << ' ' << tracf[0] << ' ' << tracf[1] << ' ' << tracf[2]
          << std::flush;
    }
  });

  vtu << std::endl;
  vtu << "            </DataArray>" << std::endl;
  vtu << "         </PointData>" << std::endl;

  vtu << "         <Cells>" << std::endl;
  vtu << "            <DataArray type=\"Int32\" Name=\"connectivity\" "
         "format=\"ascii\">"
      << std::endl;
  index = 0;
  for (size_t i = 0; i < num_cells; i++) {
    vtu << ' ' << index++ << std::flush;
  }
  vtu << std::endl;
  vtu << "            </DataArray>" << std::endl;
  vtu << "            <DataArray type=\"Int32\" Name=\"offsets\" "
         "format=\"ascii\">"
      << std::endl;
  for (size_t i = 0; i < num_cells; i++) {
    vtu << ' ' << 1 << std::flush;
  }
  vtu << std::endl;
  vtu << "            </DataArray>" << std::endl;
  vtu << "            <DataArray type=\"Int32\" Name=\"types\" "
         "format=\"ascii\">"
      << std::endl;
  for (size_t i = 0; i < num_cells; i++) {
    vtu << ' ' << 1 << std::flush;
  }
  vtu << std::endl;
  vtu << "            </DataArray>" << std::endl;
  vtu << "         </Cells>" << std::endl;
  vtu << "      </Piece>" << std::endl;
  vtu << "   </UnstructuredGrid>" << std::endl;
  vtu << "</VTKFile>" << std::endl;
}

void ParaviewExporter::ExportSummary(std::string filename,
                                     uint64_t num_iterations) {
  std::ofstream pvd(filename + ".pvd");
  auto* param = Simulation::GetActive()->GetParam();

  pvd << "<?xml version=\"1.0\"?>" << std::endl;
  pvd << "<VTKFile type=\"Collection\" version=\"0.1\" "
         "byte_order=\"LittleEndian\">"
      << std::endl;
  pvd << "<Collection>" << std::endl;
  /// iterate for all (time) steps
  for (uint64_t i = 0; i < num_iterations; i++) {
    pvd << "<DataSet timestep=\"" << (i * param->simulation_time_step_)
        << "\" group=\"\" part=\"0\" file=\"" << filename << '-' << i
        << ".vtu\">";
    pvd << std::endl;
    /// end of (time) iterations loop...
  }
  pvd << "</Collection>" << std::endl;
  pvd << "</VTKFile>" << std::endl;
}

// -----------------------------------------------------------------------------
std::unique_ptr<Exporter> ExporterFactory::GenerateExporter(ExporterType type) {
  switch (type) {
    case kBasic:
      return std::unique_ptr<Exporter>(new BasicExporter);
    case kMatlab:
      return std::unique_ptr<Exporter>(new MatlabExporter);
    case kNeuroML:
      return std::unique_ptr<Exporter>(new NeuroMLExporter);
    case kParaview:
      return std::unique_ptr<Exporter>(new ParaviewExporter);
    default:
      throw std::invalid_argument("export format not recognized");
  }
}

}  // namespace bdm
