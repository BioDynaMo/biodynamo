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


using std::fstream;
using namespace std;

namespace bdm {

class Exporter {
 public:
  template <typename daosoa>
  void ToFile(daosoa* cells, string filename) const {
    // This function exports the cell positions into a file,
    // where each line contains the 3D position of a cell in square brackets.
    const size_t n_vectors = cells->vectors();

    ofstream outfile;
    outfile.open("../demo/" + filename);

    double currPosX, currPosY, currPosZ;

    for (size_t j = 0; j < VcBackend::kVecLen; j++) {
      for (size_t i = 0; i < n_vectors; i++) {
        auto& cell = (*cells)[i];
        auto& currPos = cell.GetPosition();
        currPosX = currPos[0][j];
        currPosY = currPos[1][j];
        currPosZ = currPos[2][j];
        outfile << "[" << currPosX << "," << currPosY << "," << currPosZ << "]"
                << endl;
      }
    }

    outfile.close();

    std::cout << "created ExportFile with positions" << std::endl;
  }

  template <typename daosoa>
  void ToMatlabFile(daosoa* cells, string filename) const {
    // This function exports the cell positions into a file in Matlab format,
    // where each line contains the 3D position of a cell. An array called
    // CellPos
    // is initialized with the correct size corresponding to the number of
    // cells.
    const size_t n_vectors = cells->vectors();

    ofstream outfile;
    outfile.open("../demo/" + filename);

    double currPosX, currPosY, currPosZ;

    int nrCells = VcBackend::kVecLen * n_vectors;
    int cellID = 0;

    outfile << "CellPos = zeros(" << nrCells << "," << 3 << ");" << endl;

    for (size_t j = 0; j < VcBackend::kVecLen; j++) {
      for (size_t i = 0; i < n_vectors; i++) {
        cellID++;
        auto& cell = (*cells)[i];
        auto& currPos = cell.GetPosition();
        currPosX = currPos[0][j];
        currPosY = currPos[1][j];
        currPosZ = currPos[2][j];
        outfile << "CellPos(" << cellID << ",1:3) = [" << currPosX << ","
                << currPosY << "," << currPosZ << "];" << endl;
      }
    }

    outfile.close();

    std::cout << "created ExportFile with positions" << std::endl;
  }

  template <typename daosoa>
  void ToNeuroMLFile(daosoa* cells, string filename) const {
    // a preliminary exporter according to the NeuroML Level 3 format.
    // Currently, no axons or connectivity is present, so these information
    // will be added in the future.

    const size_t n_vectors = cells->vectors();

    ofstream outfile;
    outfile.open("../demo/" + filename);

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

    // here, the cell populations and connectivity will be
    // specified and exported
    for (size_t j = 0; j < VcBackend::kVecLen; j++) {
      for (size_t i = 0; i < n_vectors; i++) {
      }
    }

    outfile << endl;
    outfile << "</neuroml>" << endl;

    outfile.close();

    std::cout << "created NeuroML file" << std::endl;
  }
};
}   // namespace bdm
#endif  // EXPORTER_H_
