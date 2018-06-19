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

#ifndef EXPORTER_H_
#define EXPORTER_H_

#include <array>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "backend.h"
#include "log.h"
#include "param.h"
#include "bdm.h"

namespace bdm {

using std::endl;
using std::string;
using std::fstream;
using std::ofstream;
using bdm::Scalar;
using bdm::Soa;

template <typename TContainer>
class Exporter {
 public:
  /// Export the simulation state of one iteration
  /// \param cells - simulation object which should be exported
  /// \param filename
  /// \param iteration - current iteration number (=time step)
  virtual void ExportIteration(const TContainer &cells, string filename,
                               uint64_t iteration) = 0;

  /// Export the simulation summary
  /// \param filename
  /// \param num_iterations - total number of iterations
  virtual void ExportSummary(string filename, uint64_t num_iterations) = 0;
};

template <typename TContainer>
class BasicExporter : public Exporter<TContainer> {
 public:
  void ExportIteration(const TContainer &cells, string filename,
                       uint64_t iteration) override {
    ofstream outfile;
    outfile.open(filename);
    uint num_cells = cells.size();
    for (size_t i = 0; i < num_cells; i++) {
      auto &&cell = (cells)[i];
      auto curr_pos = cell.GetPosition();
      outfile << "[" << curr_pos[0] << "," << curr_pos[1] << "," << curr_pos[2]
              << "]" << endl;
    }

    outfile.close();
  }

  void ExportSummary(string filename, uint64_t num_iterations) override {}
};

template <typename TContainer>
class MatlabExporter : public Exporter<TContainer> {
 public:
  void ExportIteration(const TContainer &cells, string filename,
                       uint64_t iteration) override {
    ofstream outfile;
    outfile.open(filename);
    uint num_cells = cells.size();

    outfile << "CellPos = zeros(" << num_cells << "," << 3 << ");" << endl;

    for (size_t i = 0; i < num_cells; i++) {
      auto &&cell = (cells)[i];
      auto curr_pos = cell.GetPosition();
      outfile << "CellPos(" << i + 1 << ",1:3) = [" << curr_pos[0] << ","
              << curr_pos[1] << "," << curr_pos[2] << "];" << endl;
    }

    outfile.close();
  }

  void ExportSummary(string filename, uint64_t num_iterations) override {}
};

template <typename TContainer>
class NeuroMLExporter : public Exporter<TContainer> {
 public:
  void ExportIteration(const TContainer &cells, string filename,
                       uint64_t iteration) override {
    ofstream outfile;
    outfile.open(filename);

    const size_t num_cells = cells.size();

    string space1 = "   ";
    string space2 = "      ";
    string space3 = "         ";
    string space4 = "            ";

    outfile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
    outfile << "<neuroml xmlns      = \"http://morphml.org/neuroml/schema\""
            << endl;
    outfile << space1
            << "xmlns:xsi  = \"http://www.w3.org/2001/XMLSchema-instance\" "
            << endl;
    outfile << space1 << "xmlns:meta = \"http://morphml.org/metadata/schema\" "
            << endl;
    outfile << space1 << "xsi:schemaLocation=\"http://morphml.org/neuroml/"
                         "schema NeuroML_Level3_v1.7.1.xsd\" "
            << endl;
    outfile << space1 << "lengthUnits=\"micrometer\" " << endl;

    outfile << space1 << "<cells>" << endl;

    /// In the future, the electrophysiological properties of neurons can be
    /// inserted instead of these prespecified values
    outfile << space2 << "<cell name=\"exz_lif\"> " << endl;
    outfile << space3 << "<meta:properties>" << endl;
    outfile << space4
            << "<meta:property tag=\"PointNeuronModel\" value=\"yes\"/>"
            << endl;
    outfile << space4
            << "<meta:property tag=\"Class\"    value=\"CbLifNeuron\"/>"
            << endl;
    outfile << space4 << "<meta:property tag=\"Cm\"       value=\"5e-10\"/>"
            << endl;
    outfile << space4 << "<meta:property tag=\"Rm\"       value=\"1e8\"/>"
            << endl;
    outfile << space4 << "<meta:property tag=\"Vthresh\"  value=\"-50e-3\"/>"
            << endl;
    outfile << space4 << "<meta:property tag=\"Vresting\" value=\"-60e-3\"/>"
            << endl;
    outfile << space4 << "<meta:property tag=\"Vreset\"   value=\"-60e-3\"/>"
            << endl;
    outfile << space4 << "<meta:property tag=\"Trefract\" value=\"5e-3\"/>"
            << endl;
    outfile << space4 << "<meta:property tag=\"Vinit\"    value=\"-60e-3\"/>"
            << endl;
    outfile << space3 << "</meta:properties>" << endl;
    outfile << space2 << "</cell>" << endl;

    outfile << space2 << "<cell name=\"inh_lif\"> " << endl;
    outfile << space3 << "<meta:properties>" << endl;
    outfile << space4
            << "<meta:property tag=\"PointNeuronModel\" value=\"yes\"/>"
            << endl;
    outfile << space4
            << "<meta:property tag=\"Class\"    value=\"CbLifNeuron\"/>"
            << endl;
    outfile << space4 << "<meta:property tag=\"Cm\"       value=\"2e-10\"/>"
            << endl;
    outfile << space4 << "<meta:property tag=\"Rm\"       value=\"1e8\"/>"
            << endl;
    outfile << space4 << "<meta:property tag=\"Vthresh\"  value=\"-50e-3\"/>"
            << endl;
    outfile << space4 << "<meta:property tag=\"Vresting\" value=\"-60e-3\"/>"
            << endl;
    outfile << space4 << "<meta:property tag=\"Vreset\"   value=\"-60e-3\"/>"
            << endl;
    outfile << space4 << "<meta:property tag=\"Trefract\" value=\"5e-3\"/>"
            << endl;
    outfile << space4 << "<meta:property tag=\"Vinit\"    value=\"-60e-3\"/>"
            << endl;
    outfile << space3 << "</meta:properties>" << endl;
    outfile << space2 << "</cell>" << endl;
    outfile << space1 << "</cells>" << endl;

    /// TODO(roman): here, the cell populations and connectivity will be
    /// specified and exported, once these are included in the model
    for (size_t i = 0; i < num_cells; i++) {
    }

    outfile << endl;
    outfile << "</neuroml>" << endl;

    outfile.close();

    Log::Info("Exporter", "Created NeuroML file");
  }

  void ExportSummary(string filename, uint64_t num_iterations) override {}
};

template <typename TContainer, typename TBdmSim = BdmSim<>>
class ParaviewExporter : public Exporter<TContainer> {
 public:
  void ExportIteration(const TContainer &cells, string filename,
                       uint64_t iteration) override {
    const size_t num_cells = cells.size();
    size_t index = 0;
    std::ofstream vtu(filename + "-" + std::to_string(iteration) + ".vtu");

    vtu << "<VTKFile type=\"UnstructuredGrid\" version=\"0.1\" "
           "byte_order=\"LittleEndian\">"
        << endl;
    vtu << "   <UnstructuredGrid>" << endl;
    vtu << "      <Piece  NumberOfPoints=\"" << num_cells
        << "\" NumberOfCells=\"" << num_cells << "\">" << endl;
    vtu << "         <Points>" << endl;
    vtu << "            <DataArray type=\"Float64\" NumberOfComponents=\"3\" "
           "format=\"ascii\">"
        << endl;
    for (size_t i = 0; i < num_cells; i++) {
      auto &&cell = (cells)[i];
      auto &coord = cell.GetPosition();

      vtu << ' ' << coord[0] << ' ' << coord[1] << ' ' << coord[2]
          << std::flush;
    }
    vtu << endl;
    vtu << "            </DataArray>" << endl;
    vtu << "         </Points>" << endl;
    vtu << "         <PointData>" << endl;
    vtu << "            <DataArray type=\"Float64\" Name=\"Cell_ID\" "
           "NumberOfComponents=\"1\" format=\"ascii\">"
        << endl;
    index = 0;
    for (size_t i = 0; i < num_cells; i++) {
      vtu << ' ' << index++ << std::flush;
    }
    vtu << endl;
    vtu << "            </DataArray>" << endl;
    vtu << "            <DataArray type=\"Float64\" Name=\"Adherence\" "
           "NumberOfComponents=\"1\" format=\"ascii\">"
        << endl;
    for (size_t i = 0; i < num_cells; i++) {
      auto &&cell = (cells)[i];
      auto adhr = cell.GetAdherence();

      vtu << ' ' << adhr << std::flush;
    }
    vtu << endl;
    vtu << "            </DataArray>" << endl;
    vtu << "            <DataArray type=\"Float64\" Name=\"Diameter\" "
           "NumberOfComponents=\"1\" format=\"ascii\">"
        << endl;
    for (size_t i = 0; i < num_cells; i++) {
      auto &&cell = (cells)[i];
      auto diam = cell.GetDiameter();

      vtu << ' ' << diam << std::flush;
    }
    vtu << endl;
    vtu << "            </DataArray>" << endl;
    vtu << "            <DataArray type=\"Float64\" Name=\"Mass\" "
           "NumberOfComponents=\"1\" format=\"ascii\">"
        << endl;
    for (size_t i = 0; i < num_cells; i++) {
      auto &&cell = (cells)[i];
      auto mass = cell.GetMass();

      vtu << ' ' << mass << std::flush;
    }
    vtu << endl;
    vtu << "            </DataArray>" << endl;
    vtu << "            <DataArray type=\"Float64\" Name=\"Volume\" "
           "NumberOfComponents=\"1\" format=\"ascii\">"
        << endl;
    for (size_t i = 0; i < num_cells; i++) {
      auto &&cell = (cells)[i];
      auto volm = cell.GetVolume();

      vtu << ' ' << volm << std::flush;
    }
    vtu << endl;
    vtu << "            </DataArray>" << endl;
    vtu << "            <DataArray type=\"Float64\" Name=\"TractionForce\" "
           "NumberOfComponents=\"3\" format=\"ascii\">"
        << endl;
    for (size_t i = 0; i < num_cells; i++) {
      auto &&cell = (cells)[i];
      auto &tracf = cell.GetTractorForce();

      vtu << ' ' << tracf[0] << ' ' << tracf[1] << ' ' << tracf[2]
          << std::flush;
    }
    vtu << endl;
    vtu << "            </DataArray>" << endl;
    vtu << "         </PointData>" << endl;

    vtu << "         <Cells>" << endl;
    vtu << "            <DataArray type=\"Int32\" Name=\"connectivity\" "
           "format=\"ascii\">"
        << endl;
    index = 0;
    for (size_t i = 0; i < num_cells; i++) {
      vtu << ' ' << index++ << std::flush;
    }
    vtu << endl;
    vtu << "            </DataArray>" << endl;
    vtu << "            <DataArray type=\"Int32\" Name=\"offsets\" "
           "format=\"ascii\">"
        << endl;
    for (size_t i = 0; i < num_cells; i++) {
      vtu << ' ' << 1 << std::flush;
    }
    vtu << endl;
    vtu << "            </DataArray>" << endl;
    vtu << "            <DataArray type=\"Int32\" Name=\"types\" "
           "format=\"ascii\">"
        << endl;
    for (size_t i = 0; i < num_cells; i++) {
      vtu << ' ' << 1 << std::flush;
    }
    vtu << endl;
    vtu << "            </DataArray>" << endl;
    vtu << "         </Cells>" << endl;
    vtu << "      </Piece>" << endl;
    vtu << "   </UnstructuredGrid>" << endl;
    vtu << "</VTKFile>" << endl;
  }

  /// This function creates a .pvd file that lists the individual files across
  /// the different times.
  /// This .pvd can be read by Paraview for visualization.
  void ExportSummary(string filename, uint64_t num_iterations) override {
    std::ofstream pvd(filename + ".pvd");
    auto* param = TBdmSim::GetActive()->GetParam();

    pvd << "<?xml version=\"1.0\"?>" << endl;
    pvd << "<VTKFile type=\"Collection\" version=\"0.1\" "
           "byte_order=\"LittleEndian\">"
        << endl;
    pvd << "<Collection>" << endl;
    /// iterate for all (time) steps
    for (uint64_t i = 0; i < num_iterations; i++) {
      pvd << "<DataSet timestep=\"" << (i * param->simulation_time_step_)
          << "\" group=\"\" part=\"0\" file=\"" << filename << '-' << i
          << ".vtu\">";
      pvd << endl;
      /// end of (time) iterations loop...
    }
    pvd << "</Collection>" << endl;
    pvd << "</VTKFile>" << endl;
  }
};

enum ExporterType { kBasic, kMatlab, kNeuroML, kParaview };

class ExporterFactory {
 public:
  template <typename TContainer>
  static std::unique_ptr<Exporter<TContainer>> GenerateExporter(
      ExporterType type) {
    switch (type) {
      case kBasic:
        return std::unique_ptr<Exporter<TContainer>>(
            new BasicExporter<TContainer>);
      case kMatlab:
        return std::unique_ptr<Exporter<TContainer>>(
            new MatlabExporter<TContainer>);
      case kNeuroML:
        return std::unique_ptr<Exporter<TContainer>>(
            new NeuroMLExporter<TContainer>);
      case kParaview:
        return std::unique_ptr<Exporter<TContainer>>(
            new ParaviewExporter<TContainer>);
      default:
        throw std::invalid_argument("export format not recognized");
    }
  }
};
}  // namespace bdm
#endif  // EXPORTER_H_
