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

  std::vector<Cell> cells;
  cells.push_back(cell1);
  cells.push_back(cell2);

  Exporter exporter;

  // Test the standard file exporter
  exporter.ToFile(&cells, "TestExporter.dat");
  std::ifstream t;
  std::stringstream buffer;
  t.open("TestExporter.dat");
  std::string line;
  std::getline(t, line);
  EXPECT_EQ("[0.5,1,0]", line);
  std::getline(t, line);
  EXPECT_EQ("[-5,5,0.9]", line);
  std::getline(t, line);
  EXPECT_EQ("", line);
  t.close();
  remove("TestExporter.dat");

  // Test the Matlab file exporter
  exporter.ToMatlabFile(&cells, "TestMatlabExporter.m");
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

  // Test the Paraview exporter
  exporter.CreatePVDFile("TestResultsParaview", 1, 1.0);
  exporter.ToVTUFile(&cells, "TestResultsParaview", 1);
  t.open("TestResultsParaview.pvd");
  std::getline(t, line);
  EXPECT_EQ("<?xml version=\"1.0\"?>", line);
  std::getline(t, line);
  EXPECT_EQ(
      "<VTKFile type=\"Collection\" version=\"0.1\" "
      "byte_order=\"LittleEndian\">",
      line);
  t.close();

  t.open("TestResultsParaview-1.vtu");
  std::getline(t, line);
  EXPECT_EQ(
      "<VTKFile type=\"UnstructuredGrid\" version=\"0.1\" "
      "byte_order=\"LittleEndian\">",
      line);
  std::getline(t, line);
  EXPECT_EQ("   <UnstructuredGrid>", line);
  std::getline(t, line);
  EXPECT_EQ("      <Piece  NumberOfPoints=\"2\" NumberOfCells=\"2\">", line);
  std::getline(t, line);
  std::getline(t, line);
  std::getline(t, line);
  EXPECT_EQ(" 0.5 1 0 -5 5 0.9", line);
  t.close();
}
}  // namespace bdm
