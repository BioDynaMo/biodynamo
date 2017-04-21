#ifndef EXPORTER_H_
#define EXPORTER_H_

#include <array>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "backend.h"
#include "daosoa.h"
#include "default_force.h"
#include "inline_vector.h"
#include "math_util.h"
#include "param.h"

using std::cout;
using std::endl;
using std::string;
using std::fstream;
using std::ofstream;

namespace bdm {

class Exporter {
 public:
  /// This function exports the cell positions into a file,
  /// where each line contains the 3D position of a cell in square brackets.
  template <typename daosoa>
  void ToFile(const daosoa& cells, string filename) const {
    const size_t num_vectors = cells.vectors();

    ofstream outfile;
    outfile.open(filename);

    double curr_pos_x, curr_pos_y, curr_pos_z;

    for (size_t i = 0; i < num_vectors; i++) {
      for (size_t j = 0; j < cells[i].Size(); j++) {
        auto& cell = cells[i];
        auto& curr_pos = cell.GetPosition();
        curr_pos_x = curr_pos[0][j];
        curr_pos_y = curr_pos[1][j];
        curr_pos_z = curr_pos[2][j];
        outfile << "[" << curr_pos_x << "," << curr_pos_y << "," << curr_pos_z
                << "]" << endl;
      }
    }

    outfile.close();

    std::cout << "created ExportFile with positions" << std::endl;
  }

  /// This function exports the cell positions into a file in Matlab format,
  /// where each line contains the 3D position of a cell. An array called
  /// CellPos
  /// is initialized with the correct size corresponding to the number of
  /// cells.
  template <typename daosoa>
  void ToMatlabFile(const daosoa& cells, string filename) const {
    const size_t num_vectors = cells.vectors();

    ofstream outfile;
    outfile.open(filename);

    double curr_pos_x, curr_pos_y, curr_pos_z;

    int num_cells = VcBackend::kVecLen * num_vectors;
    int cell_id = 0;

    outfile << "CellPos = zeros(" << num_cells << "," << 3 << ");" << endl;

    for (size_t i = 0; i < num_vectors; i++) {
      for (size_t j = 0; j < cells[i].Size(); j++) {
        cell_id++;
        auto& cell = cells[i];
        auto& curr_pos = cell.GetPosition();
        curr_pos_x = curr_pos[0][j];
        curr_pos_y = curr_pos[1][j];
        curr_pos_z = curr_pos[2][j];
        outfile << "CellPos(" << cell_id << ",1:3) = [" << curr_pos_x << ","
                << curr_pos_y << "," << curr_pos_z << "];" << endl;
      }
    }

    outfile.close();

    std::cout << "created ExportFile with positions" << std::endl;
  }

  /// a preliminary exporter according to the NeuroML Level 3 format.
  /// Currently, no axons or connectivity is present, so these information
  /// will be added in the future.
  template <typename daosoa>
  void ToNeuroMLFile(const daosoa& cells, string filename) const {
    const size_t num_vectors = cells.vectors();

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
    // specified and exported, onece these are included in the model

    for (size_t i = 0; i < num_vectors; i++) {
      for (size_t j = 0; j < cells[i].Size(); j++) {
      }
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
  template <typename daosoa>
  void ToVTUFile(const daosoa& cells, string filename,
                 size_t iteration_index) const {
    const size_t num_vectors = cells.vectors();
    const size_t num_cells = VcBackend::kVecLen * num_vectors;
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
    for (size_t i = 0; i < num_vectors; i++) {
      auto& cell = cells[i];
      auto& coord = cell.GetPosition();
      for (size_t j = 0; j < cell.Size(); j++) {
        vtu << ' ' << coord[0][j] << ' ' << coord[1][j] << ' ' << coord[2][j]
            << std::flush;
      }
    }
    vtu << std::endl;
    vtu << "            </DataArray>" << std::endl;
    vtu << "         </Points>" << std::endl;
    vtu << "         <PointData>" << std::endl;
    vtu << "            <DataArray type=\"Float64\" Name=\"Cell_ID\" "
           "NumberOfComponents=\"1\" format=\"ascii\">"
        << std::endl;
    index = 0;
    for (size_t i = 0; i < num_vectors; i++) {
      auto& cell = cells[i];
      for (size_t j = 0; j < cell.Size(); j++)
        vtu << ' ' << index++ << std::flush;
    }
    vtu << std::endl;
    vtu << "            </DataArray>" << std::endl;
    vtu << "            <DataArray type=\"Float64\" Name=\"Adherence\" "
           "NumberOfComponents=\"1\" format=\"ascii\">"
        << std::endl;
    for (size_t i = 0; i < num_vectors; i++) {
      auto& cell = cells[i];
      auto& adhr = cell.GetAdherence();
      for (size_t j = 0; j < cell.Size(); j++)
        vtu << ' ' << adhr[j] << std::flush;
    }
    vtu << std::endl;
    vtu << "            </DataArray>" << std::endl;
    vtu << "            <DataArray type=\"Float64\" Name=\"Diameter\" "
           "NumberOfComponents=\"1\" format=\"ascii\">"
        << std::endl;
    for (size_t i = 0; i < num_vectors; i++) {
      auto& cell = cells[i];
      auto& diam = cell.GetDiameter();
      for (size_t j = 0; j < cell.Size(); j++)
        vtu << ' ' << diam[j] << std::flush;
    }
    vtu << std::endl;
    vtu << "            </DataArray>" << std::endl;
    vtu << "            <DataArray type=\"Float64\" Name=\"Mass\" "
           "NumberOfComponents=\"1\" format=\"ascii\">"
        << std::endl;
    for (size_t i = 0; i < num_vectors; i++) {
      auto& cell = cells[i];
      auto& mass = cell.GetMass();
      for (size_t j = 0; j < cell.Size(); j++)
        vtu << ' ' << mass[j] << std::flush;
    }
    vtu << std::endl;
    vtu << "            </DataArray>" << std::endl;
    vtu << "            <DataArray type=\"Float64\" Name=\"Volume\" "
           "NumberOfComponents=\"1\" format=\"ascii\">"
        << std::endl;
    for (size_t i = 0; i < num_vectors; i++) {
      auto& cell = cells[i];
      auto& volm = cell.GetVolume();
      for (size_t j = 0; j < cell.Size(); j++)
        vtu << ' ' << volm[j] << std::flush;
    }
    vtu << std::endl;
    vtu << "            </DataArray>" << std::endl;
    vtu << "            <DataArray type=\"Float64\" Name=\"TractionForce\" "
           "NumberOfComponents=\"3\" format=\"ascii\">"
        << std::endl;
    for (size_t i = 0; i < num_vectors; i++) {
      auto& cell = cells[i];
      auto& tracf = cell.GetTractorForce();
      for (size_t j = 0; j < cell.Size(); j++)
        vtu << ' ' << tracf[0][j] << ' ' << tracf[1][j] << ' ' << tracf[2][j]
            << std::flush;
    }
    vtu << std::endl;
    vtu << "            </DataArray>" << std::endl;
    vtu << "         </PointData>" << std::endl;
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
    for (size_t i = 0; i < num_vectors; i++) {
      auto& cell = cells[i];
      for (size_t j = 0; j < cell.Size(); j++)
        vtu << ' ' << index++ << std::flush;
    }
    vtu << std::endl;
    vtu << "            </DataArray>" << std::endl;
    vtu << "            <DataArray type=\"Int32\" Name=\"offsets\" "
           "format=\"ascii\">"
        << std::endl;
    for (size_t i = 0; i < num_vectors; i++) {
      auto& cell = cells[i];
      for (size_t j = 0; j < cell.Size(); j++) vtu << ' ' << 1 << std::flush;
    }
    vtu << std::endl;
    vtu << "            </DataArray>" << std::endl;
    vtu << "            <DataArray type=\"Int32\" Name=\"types\" "
           "format=\"ascii\">"
        << std::endl;
    for (size_t i = 0; i < num_vectors; i++) {
      auto& cell = cells[i];
      for (size_t j = 0; j < cell.Size(); j++) vtu << ' ' << 1 << std::flush;
    }
    vtu << std::endl;
    vtu << "            </DataArray>" << std::endl;
    vtu << "         </Cells>" << std::endl;
    vtu << "      </Piece>" << std::endl;
    vtu << "   </UnstructuredGrid>" << std::endl;
    vtu << "</VTKFile>" << std::endl;
  }
};
}  // namespace bdm
#endif  // EXPORTER_H_
