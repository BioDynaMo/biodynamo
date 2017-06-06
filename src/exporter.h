#ifndef EXPORTER_H_
#define EXPORTER_H_

#include <array>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "backend.h"
#include "cell.h"
#include "displacement_op.h"
#include "dividing_cell_op.h"
#include "exporter.h"
#include "neighbor_nanoflann_op.h"
#include "neighbor_op.h"
#include "resource_manager.h"
#include "scheduler.h"
#include "timing.h"
#include "timing_aggregator.h"


namespace bdm {

using std::cout;
using std::endl;
using std::string;
using std::fstream;
using std::ofstream;
using bdm::Cell;
using bdm::Scalar;
using bdm::Soa;
using bdm::Timing;
using bdm::TimingAggregator;


class Exporter {
public:
  /// This function exports the cell positions into a file,
  /// where each line contains the 3D position of a cell in square brackets.
  template <typename TContainer>
  void ToFile(const TContainer *cells, string filename) const {
    ofstream outfile;
    outfile.open(filename);

    for (size_t i = 0; i < cells->size(); i++) {
      auto &&cell = (*cells)[i];
      auto curr_pos = cell.GetPosition();
      outfile << "[" << curr_pos[0] << "," << curr_pos[1] << "," << curr_pos[2]
              << "]" << endl;
    }

    outfile.close();

    std::cout << "created ExportFile with positions" << std::endl;
  }

  /// This function exports the cell positions into a file in Matlab format,
  /// where each line contains the 3D position of a cell. An array called
  /// CellPos
  /// is initialized with the correct size corresponding to the number of
  /// cells.
  template <typename TContainer>
  void ToMatlabFile(const TContainer *cells, string filename) const {
    ofstream outfile;
    outfile.open(filename);

    int num_cells = cells->size();

    outfile << "CellPos = zeros(" << num_cells << "," << 3 << ");" << endl;

    for (size_t i = 0; i < cells->size(); i++) {
      auto &&cell = (*cells)[i];
      auto curr_pos = cell.GetPosition();
      outfile << "CellPos(" << i + 1 << ",1:3) = [" << curr_pos[0] << ","
              << curr_pos[1] << "," << curr_pos[2] << "];" << endl;
    }

    outfile.close();

    std::cout << "created ExportFile with positions" << std::endl;
  }

  /// a preliminary exporter according to the NeuroML Level 3 format.
  /// Currently, no axons or connectivity is present, so these information
  /// will be added in the future.
  template <typename TContainer>
  void ToNeuroMLFile(const TContainer *cells, string filename) const {
    ofstream outfile;
    outfile.open(filename);

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

    // In the future, the electrophysiological properties of neurons can be
    // inserted instead of these prespecified values
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

    // TODO(roman): here, the cell populations and connectivity will be
    // specified and exported, once these are included in the model
    for (size_t i = 0; i < cells->size(); i++) {
    }

    outfile << endl;
    outfile << "</neuroml>" << endl;

    outfile.close();

    std::cout << "created NeuroML file" << std::endl;
  }

  /// This function creates a .pvd file that lists the individual files across
  /// the different times.
  /// This .pvd can be read by Paraview for visualization.
  void CreatePVDFile(string filename, int iterations, double increment) {
    std::ofstream pvd(filename + ".pvd");

    pvd << "<?xml version=\"1.0\"?>" << std::endl;
    pvd << "<VTKFile type=\"Collection\" version=\"0.1\" "
           "byte_order=\"LittleEndian\">"
        << std::endl;
    pvd << "<Collection>" << std::endl;
    // iterate for all (time) steps
    for (int i = 0; i < iterations; i++) {
      pvd << "<DataSet timestep=\"" << (i * increment)
          << "\" group=\"\" part=\"0\" file=\"" << filename << '-' << i
          << ".vtu\">";
      pvd << std::endl;
      // end of (time) iterations loop...
    }
    pvd << "</Collection>" << std::endl;
    pvd << "</VTKFile>" << std::endl;
  }

  /// This function exports the cell positions as well as properties into a .vtu
  /// file,
  /// which can be read by Paraview for visualization.
  template <typename TContainer>
  void ToVTUFile(const TContainer *cells, string filename,
                 size_t iteration_index) const {
    const size_t num_cells = cells->size();
    size_t index = 0;
    std::ofstream vtu(filename + "-" + std::to_string(iteration_index) +
                      ".vtu");

    vtu << "<VTKFile type=\"UnstructuredGrid\" version=\"0.1\" "
           "byte_order=\"LittleEndian\">"
        << std::endl;
    vtu << "   <UnstructuredGrid>" << std::endl;
    vtu << "      <Piece  NumberOfPoints=\"" << num_cells
        << "\" NumberOfCells=\"" << num_cells << "\">" << std::endl;
    vtu << "         <Points>" << std::endl;
    vtu << "            <DataArray type=\"Float64\" NumberOfComponents=\"3\" "
           "format=\"ascii\">"
        << std::endl;
    for (size_t i = 0; i < num_cells; i++) {
      auto &&cell = (*cells)[i];
      auto &coord = cell.GetPosition();

      vtu << ' ' << coord[0] << ' ' << coord[1] << ' ' << coord[2]
          << std::flush;
    }
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
    for (size_t i = 0; i < num_cells; i++) {
      auto &&cell = (*cells)[i];
      auto adhr = cell.GetAdherence();

      vtu << ' ' << adhr << std::flush;
    }
    vtu << std::endl;
    vtu << "            </DataArray>" << std::endl;
    vtu << "            <DataArray type=\"Float64\" Name=\"Diameter\" "
           "NumberOfComponents=\"1\" format=\"ascii\">"
        << std::endl;
    for (size_t i = 0; i < num_cells; i++) {
      auto &&cell = (*cells)[i];
      auto diam = cell.GetDiameter();

      vtu << ' ' << diam << std::flush;
    }
    vtu << std::endl;
    vtu << "            </DataArray>" << std::endl;
    vtu << "            <DataArray type=\"Float64\" Name=\"Mass\" "
           "NumberOfComponents=\"1\" format=\"ascii\">"
        << std::endl;
    for (size_t i = 0; i < num_cells; i++) {
      auto &&cell = (*cells)[i];
      auto mass = cell.GetMass();

      vtu << ' ' << mass << std::flush;
    }
    vtu << std::endl;
    vtu << "            </DataArray>" << std::endl;
    vtu << "            <DataArray type=\"Float64\" Name=\"Volume\" "
           "NumberOfComponents=\"1\" format=\"ascii\">"
        << std::endl;
    for (size_t i = 0; i < num_cells; i++) {
      auto &&cell = (*cells)[i];
      auto volm = cell.GetVolume();

      vtu << ' ' << volm << std::flush;
    }
    vtu << std::endl;
    vtu << "            </DataArray>" << std::endl;
    vtu << "            <DataArray type=\"Float64\" Name=\"TractionForce\" "
           "NumberOfComponents=\"3\" format=\"ascii\">"
        << std::endl;
    for (size_t i = 0; i < num_cells; i++) {
      auto &&cell = (*cells)[i];
      auto &tracf = cell.GetTractorForce();

      vtu << ' ' << tracf[0] << ' ' << tracf[1] << ' ' << tracf[2]
          << std::flush;
    }
    vtu << std::endl;
    vtu << "            </DataArray>" << std::endl;
    vtu << "         </PointData>" << std::endl;

    // roman: this is relevant for Int32 format, keeping in case
    // required in the future
    // vtu << "         <CellData>" << std::endl;
    // vtu << "            <DataArray type=\"Int32\" Name=\"cell_ID\"
    // NumberOfComponents=\"1\" format=\"ascii\">" << std::endl;
    // index = 0;
    // for (size_t i=0; i<num_vectors; i++) {
    //   auto& cell = cells[i];
    //   for (size_t j=0; j<cell.Size(); j++)
    //     vtu << ' ' << index++ << std::flush;
    // }
    // vtu << std::endl;
    // vtu << "            </DataArray>" << std::endl;
    // vtu << "         </CellData>" << std::endl;

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
};
} // namespace bdm
#endif // EXPORTER_H_
