#ifndef EXPORTER_H_
#define EXPORTER_H_

#include <array>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "backend.h"
#include "default_force.h"
#include "inline_vector.h"
#include "math_util.h"
#include "param.h"

namespace bdm {

using std::cout;
using std::endl;
using std::string;
using std::fstream;
using std::ofstream;

class Exporter {
 public:
  /// This function exports the cell positions into a file,
  /// where each line contains the 3D position of a cell in square brackets.
  template <typename TContainer>
  void ToFile(const TContainer& cells, string filename) const {
    ofstream outfile;
    outfile.open(filename);

    for (size_t i = 0; i < cells.size(); i++) {
      auto& cell = cells[i];
      auto& curr_pos = cell.GetPosition();
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
  void ToMatlabFile(const TContainer& cells, string filename) const {
    ofstream outfile;
    outfile.open(filename);

    int num_cells = cells.size();

    outfile << "CellPos = zeros(" << num_cells << "," << 3 << ");" << endl;

    for (size_t i = 0; i < cells.size(); i++) {
      auto& cell = cells[i];
      auto& curr_pos = cell.GetPosition();
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
  void ToNeuroMLFile(const TContainer& cells, string filename) const {
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
    for (size_t i = 0; i < cells.size(); i++) {
    }

    outfile << endl;
    outfile << "</neuroml>" << endl;

    outfile.close();

    std::cout << "created NeuroML file" << std::endl;
  }
};
}  // namespace bdm
#endif  // EXPORTER_H_
