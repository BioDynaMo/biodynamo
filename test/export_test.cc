#include "cell.h"
#include "exporter.h"
#include "gtest/gtest.h"

namespace bdm {

TEST(ExportTest, ConductExportToFile) {
  using real_v = VcBackend::real_v;
  using real_t = real_v::value_type;
  if (real_v::Size < 2) {
    FAIL() << "Backend must at least support two elements for this test";
  }

  // set up cells and their positions
  real_v diameter((const real_t[]){10, 10});
  std::array<real_v, 3> position = {real_v((const real_t[]){0.5, -5}),
                                    real_v((const real_t[]){1, 5}),
                                    real_v((const real_t[]){0, 0.9})};
  Cell<VcBackend> cell(diameter);
  cell.SetPosition(position);
  daosoa<Cell, VcBackend> cells;
  cells.push_back(cell);

  Exporter exporter;

  // Test the standard file exporter
  exporter.ToFile(&cells, "TestExporter.dat");
  std::ifstream t;
  std::stringstream buffer;
  t.open("../demo/TestExporter.dat");
  std::string line;
  std::getline(t, line);
  EXPECT_EQ(line, "[0.5,1,0]");
  std::getline(t, line);
  EXPECT_EQ(line, "[-5,5,0.9]");
  t.close();
  remove("../demo/TestExporter.dat");

  // Test the Matlab file exporter
  exporter.ToMatlabFile(&cells, "TestMatlabExporter.m");
  t.open("../demo/TestMatlabExporter.m");
  std::getline(t, line);
  EXPECT_EQ(line, "CellPos = zeros(2,3);");
  std::getline(t, line);
  EXPECT_EQ(line, "CellPos(1,1:3) = [0.5,1,0];");
  t.close();
  remove("../demo/TestMatlabExporter.m");

  // Test the NeuroML file exporter
  exporter.ToNeuroMLFile(&cells, "TestNeuroMLExporter.xml");
  t.open("../demo/TestNeuroMLExporter.xml");
  std::getline(t, line);
  EXPECT_EQ(line, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
  t.close();
  remove("../demo/TestNeuroMLExporter.xml");
}
}
