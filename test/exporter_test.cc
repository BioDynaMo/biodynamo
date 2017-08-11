#include "exporter.h"
#include "cell.h"
#include "gtest/gtest.h"
#include "test_util.h"

namespace bdm {

TEST(ExportTest, ConductExportToFile) {
  // set up cells and their positions
  Cell cell1;
  cell1.SetPosition({0.5, 1, 0});
  cell1.SetDiameter(10);
  Cell cell2;
  cell2.SetPosition({-5, 5, 0.9});
  cell2.SetDiameter(10);

  auto cells = Cell::NewEmptySoa();
  cells.push_back(cell1);
  cells.push_back(cell2);

  ExporterFactory exp_fac;

  /// Test the standard file exporter
  auto exp_basic = exp_fac.GenerateExporter<SoaCell>("basic");
  exp_basic->ToFile(cells, "TestBasicExporter.dat");
  std::ifstream t;
  std::stringstream buffer;
  t.open("TestBasicExporter.dat");
  std::string line;
  std::getline(t, line);
  EXPECT_EQ("[0.5,1,0]", line);
  std::getline(t, line);
  EXPECT_EQ("[-5,5,0.9]", line);
  std::getline(t, line);
  EXPECT_EQ("", line);
  t.close();
  remove("TestBasicExporter.dat");

  /// Test the Matlab file exporter
  auto exp_matlab = exp_fac.GenerateExporter<SoaCell>("matlab");
  exp_matlab->ToFile(cells, "TestMatlabExporter.m");
  t.open("TestMatlabExporter.m");
  std::getline(t, line);
  EXPECT_EQ("CellPos = zeros(2,3);", line);
  std::getline(t, line);
  EXPECT_EQ("CellPos(1,1:3) = [0.5,1,0];", line);
  std::getline(t, line);
  EXPECT_EQ("CellPos(2,1:3) = [-5,5,0.9];", line);
  std::getline(t, line);
  EXPECT_EQ("", line);
  t.close();
  remove("TestMatlabExporter.m");

  /// Test the NeuroML file exporter
  auto exp_neuroml = exp_fac.GenerateExporter<SoaCell>("neuroml");
  exp_neuroml->ToFile(cells, "TestNeuroMLExporter.m");
  t.open("TestNeuroMLExporter.m");
  std::getline(t, line);
  EXPECT_EQ("<?xml version=\"1.0\" encoding=\"UTF-8\"?>", line);
  for (int i = 0; i < 5; ++i) {
    std::getline(t, line);
  }
  EXPECT_EQ("   lengthUnits=\"micrometer\" ", line);
  for (int i = 0; i < 30; ++i) {
    std::getline(t, line);
  }
  EXPECT_EQ("</neuroml>", line);
  t.close();
  remove("TestNeuroMLExporter.m");

  /// Test the Paraview exporter
  auto exp_paraview = exp_fac.GenerateExporter<SoaCell>("paraview");
  exp_paraview->CreatePVDFile("TestResultsParaview", 1, 1.0);
  exp_paraview->ToFile(cells, "TestResultsParaview");
  t.open("TestResultsParaview.pvd");
  std::getline(t, line);
  EXPECT_EQ("<?xml version=\"1.0\"?>", line);
  std::getline(t, line);
  EXPECT_EQ(
      "<VTKFile type=\"Collection\" version=\"0.1\" "
      "byte_order=\"LittleEndian\">",
      line);
  std::getline(t, line);
  EXPECT_EQ("<Collection>", line);
  std::getline(t, line);
  EXPECT_EQ(
      "<DataSet timestep=\"0\" group=\"\" part=\"0\" "
      "file=\"TestResultsParaview-0.vtu\">",
      line);
  std::getline(t, line);
  EXPECT_EQ("</Collection>", line);
  remove("TestResultsParaview.pvd");
  t.close();
  remove("TestResultsParaview.pvd");

  t.open("TestResultsParaview-1.vtu");
  std::getline(t, line);
  EXPECT_EQ(
      "<VTKFile type=\"UnstructuredGrid\" version=\"0.1\" "
      "byte_order=\"LittleEndian\">",
      line);
  for (int i = 0; i < 5; ++i) {
    std::getline(t, line);
  }
  EXPECT_EQ(" 0.5 1 0 -5 5 0.9", line);
  for (int i = 0; i < 20; ++i) {
    std::getline(t, line);
  }
  EXPECT_EQ(" 0 0 0 0 0 0", line);
  for (int i = 0; i < 16; ++i) {
    std::getline(t, line);
  }
  EXPECT_EQ("</VTKFile>", line);
  remove("TestResultsParaview-1.vtu");
}
}  // namespace bdm
