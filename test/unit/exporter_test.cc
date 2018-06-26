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

#include "exporter.h"
#include "cell.h"
#include "gtest/gtest.h"
#include "simulation_implementation.h"
#include "unit/default_ctparam.h"
#include "unit/test_util.h"

namespace bdm {

TEST(ExportTest, ExportToFile) {
  Simulation<> simulation(TEST_NAME);

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

  /// Test the standard file exporter
  auto exp_basic = ExporterFactory::GenerateExporter<SoaCell>(kBasic);
  exp_basic->ExportIteration(cells, "TestBasicExporter.dat", 0);
  std::ifstream ifs;
  ifs.open("TestBasicExporter.dat");
  std::string line;
  std::getline(ifs, line);
  EXPECT_EQ("[0.5,1,0]", line);
  std::getline(ifs, line);
  EXPECT_EQ("[-5,5,0.9]", line);
  EXPECT_FALSE(std::getline(ifs, line));
  EXPECT_EQ("", line);
  ifs.close();
  remove("TestBasicExporter.dat");

  /// Test the Matlab file exporter
  auto exp_matlab = ExporterFactory::GenerateExporter<SoaCell>(kMatlab);
  exp_matlab->ExportIteration(cells, "TestMatlabExporter.m", 0);
  ifs.open("TestMatlabExporter.m");
  std::getline(ifs, line);
  EXPECT_EQ("CellPos = zeros(2,3);", line);
  std::getline(ifs, line);
  EXPECT_EQ("CellPos(1,1:3) = [0.5,1,0];", line);
  std::getline(ifs, line);
  EXPECT_EQ("CellPos(2,1:3) = [-5,5,0.9];", line);
  EXPECT_FALSE(std::getline(ifs, line));
  EXPECT_EQ("", line);
  ifs.close();
  remove("TestMatlabExporter.m");

  /// Test the NeuroML file exporter
  auto exp_neuroml = ExporterFactory::GenerateExporter<SoaCell>(kNeuroML);
  exp_neuroml->ExportIteration(cells, "TestNeuroMLExporter.xml", 0);
  ifs.open("TestNeuroMLExporter.xml");
  std::getline(ifs, line);
  EXPECT_EQ("<?xml version=\"1.0\" encoding=\"UTF-8\"?>", line);
  for (int i = 0; i < 5; ++i) {
    std::getline(ifs, line);
  }
  EXPECT_EQ("   lengthUnits=\"micrometer\" ", line);
  for (int i = 0; i < 30; ++i) {
    std::getline(ifs, line);
  }
  EXPECT_EQ("</neuroml>", line);
  EXPECT_FALSE(std::getline(ifs, line));
  EXPECT_EQ("", line);
  ifs.close();
  remove("TestNeuroMLExporter.xml");

  /// Test the Paraview exporter
  auto exp_paraview = ExporterFactory::GenerateExporter<SoaCell>(kParaview);
  exp_paraview->ExportIteration(cells, "TestResultsParaview", 0);
  exp_paraview->ExportSummary("TestResultsParaview", 1);
  ifs.open("TestResultsParaview.pvd");
  std::getline(ifs, line);
  EXPECT_EQ("<?xml version=\"1.0\"?>", line);
  std::getline(ifs, line);
  EXPECT_EQ(
      "<VTKFile type=\"Collection\" version=\"0.1\" "
      "byte_order=\"LittleEndian\">",
      line);
  std::getline(ifs, line);
  EXPECT_EQ("<Collection>", line);
  std::getline(ifs, line);
  EXPECT_EQ(
      "<DataSet timestep=\"0\" group=\"\" part=\"0\" "
      "file=\"TestResultsParaview-0.vtu\">",
      line);
  std::getline(ifs, line);
  EXPECT_EQ("</Collection>", line);
  std::getline(ifs, line);
  EXPECT_EQ("</VTKFile>", line);
  EXPECT_FALSE(std::getline(ifs, line));
  EXPECT_EQ("", line);
  ifs.close();
  remove("TestResultsParaview.pvd");

  ifs.open("TestResultsParaview-0.vtu");
  std::getline(ifs, line);
  EXPECT_EQ(
      "<VTKFile type=\"UnstructuredGrid\" version=\"0.1\" "
      "byte_order=\"LittleEndian\">",
      line);
  for (int i = 0; i < 5; ++i) {
    std::getline(ifs, line);
  }
  EXPECT_EQ(" 0.5 1 0 -5 5 0.9", line);
  for (int i = 0; i < 5; ++i) {
    std::getline(ifs, line);
  }
  EXPECT_EQ(" 0 1", line);
  for (int i = 0; i < 3; ++i) {
    std::getline(ifs, line);
  }
  EXPECT_EQ(" 0 0", line);
  for (int i = 0; i < 3; ++i) {
    std::getline(ifs, line);
  }
  EXPECT_EQ(" 10 10", line);
  for (int i = 0; i < 3; ++i) {
    std::getline(ifs, line);
  }
  EXPECT_EQ(" 523.599 523.599", line);
  for (int i = 0; i < 3; ++i) {
    std::getline(ifs, line);
  }
  EXPECT_EQ(" 523.599 523.599", line);
  for (int i = 0; i < 3; ++i) {
    std::getline(ifs, line);
  }
  EXPECT_EQ(" 0 0 0 0 0 0", line);
  for (int i = 0; i < 4; ++i) {
    std::getline(ifs, line);
  }
  EXPECT_EQ(
      "            <DataArray type=\"Int32\" Name=\"connectivity\" "
      "format=\"ascii\">",
      line);
  for (int i = 0; i < 4; ++i) {
    std::getline(ifs, line);
  }
  EXPECT_EQ(" 1 1", line);
  for (int i = 0; i < 3; ++i) {
    std::getline(ifs, line);
  }
  EXPECT_EQ(" 1 1", line);
  for (int i = 0; i < 5; ++i) {
    std::getline(ifs, line);
  }
  EXPECT_EQ("</VTKFile>", line);
  EXPECT_FALSE(std::getline(ifs, line));
  EXPECT_EQ("", line);
  ifs.close();
  remove("TestResultsParaview-0.vtu");
}
}  // namespace bdm
