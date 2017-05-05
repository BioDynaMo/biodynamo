#include "gtest/gtest.h"

#include <TFile.h>

#include "cell.h"
#include "displacement_op.h"
#include "dividing_cell_op.h"
#include "io_util.h"
#include "test_util.h"

#define ROOTFILE "bdmFile.root"

namespace bdm {

TEST(IOTest, InlineVector) {
  InlineVector<int, 8> neighbor;
  for (int i = 0; i < 15; i++) {
    neighbor.push_back(i);
  }

  OneElementArray<InlineVector<int, 8>> aoi_scalar(neighbor);
  std::vector<InlineVector<int, 8>> aoi_vector;
  for (int i = 0; i < 4; i++) {
    aoi_vector.push_back(neighbor);
  }

  WritePersistentObject(ROOTFILE, "InlineVector", neighbor, "RECREATE");
  WritePersistentObject(ROOTFILE, "S_InlineVector", aoi_scalar, "UPDATE");
  WritePersistentObject(ROOTFILE, "V_InlineVector", aoi_vector, "UPDATE");

  InlineVector<int, 8>* neighbor_r;

  OneElementArray<InlineVector<int, 8>>* aoi_scalar_r;
  std::vector<InlineVector<int, 8>>* aoi_vector_r;

  GetPersistentObject(ROOTFILE, "InlineVector", neighbor_r);
  GetPersistentObject(ROOTFILE, "S_InlineVector", aoi_scalar_r);
  GetPersistentObject(ROOTFILE, "V_InlineVector", aoi_vector_r);

  EXPECT_EQ(neighbor.size(), neighbor_r->size());

  if (!(neighbor == (*neighbor_r)))
    FAIL();

  if (!(aoi_scalar[0] == (*aoi_scalar_r)[0]))
    FAIL();

  for (size_t i = 0; i < aoi_vector.size(); i++) {
    if (!(aoi_vector[i] == (*aoi_vector_r)[i]))
      FAIL();
  }
}

template <typename T>
void RunTestDivCell(T* cells) {
  cells->push_back(Cell<>(19.0));
  cells->push_back(Cell<>(41.0));

  DividingCellOp op;
  op.Compute(cells);

  WritePersistentObject(ROOTFILE, "Cells", *cells, "RECREATE");

  T* cells_r;
  GetPersistentObject(ROOTFILE, "Cells", cells_r);

  EXPECT_NEAR(19.005288996600001, (*cells_r)[0].GetDiameter(),
              abs_error<double>::value);
  EXPECT_NEAR(41, (*cells_r)[1].GetDiameter(), abs_error<double>::value);

  EXPECT_NEAR(3594.3640018287319, (*cells_r)[0].GetVolume(),
              abs_error<double>::value);
  EXPECT_NEAR(36086.951213010347, (*cells_r)[1].GetVolume(),
              abs_error<double>::value);
}

TEST(IOTest, DividingCellAos) {
  std::vector<Cell<Scalar>> cells;
  RunTestDivCell(&cells);
}

TEST(IOTest, DividingCellSoa) {
  Cell<Soa> cells;
  RunTestDivCell(&cells);
}

template <typename T>
void RunTestDispCell(T* cells) {
  // Cell 1
  Cell<> cell;
  cell.SetAdherence(0.3);
  cell.SetDiameter(9);
  cell.SetMass(1.4);
  cell.SetPosition({0, 0, 0});
  cell.SetMassLocation({0, 0, 0});
  // cell.SetTractorForce(tractor_force);
  InlineVector<int, 8> neighbor_1;
  neighbor_1.push_back(1);
  cell.SetNeighbors(neighbor_1);
  cells->push_back(cell);

  // Cell 2
  cell.SetAdherence(0.4);
  cell.SetDiameter(11);
  cell.SetMass(1.1);
  cell.SetPosition({0, 5, 0});
  cell.SetMassLocation({0, 5, 0});
  // cell.SetTractorForce(tractor_force);
  InlineVector<int, 8> neighbor_2;
  neighbor_2.push_back(0);
  cell.SetNeighbors(neighbor_2);
  cells->push_back(cell);

  // execute operation
  DisplacementOp op;
  op.Compute(cells);

  WritePersistentObject(ROOTFILE, "Cells", *cells, "RECREATE");

  T* cells_r;
  GetPersistentObject(ROOTFILE, "Cells", cells_r);

  // check results
  // cell 1
  auto final_position = (*cells_r)[0].GetPosition();
  EXPECT_NEAR(0, final_position[0], abs_error<double>::value);
  EXPECT_NEAR(-0.07797206232558615, final_position[1],
              abs_error<double>::value);
  EXPECT_NEAR(0, final_position[2], abs_error<double>::value);
  // cell 2
  final_position = (*cells_r)[1].GetPosition();
  EXPECT_NEAR(0, final_position[0], abs_error<double>::value);
  EXPECT_NEAR(5.0992371702325645, final_position[1], abs_error<double>::value);
  EXPECT_NEAR(0, final_position[2], abs_error<double>::value);

  // check if tractor_force has been reset to zero
  // cell 1
  auto final_tf = (*cells_r)[0].GetTractorForce();
  EXPECT_NEAR(0, final_tf[0], abs_error<double>::value);
  EXPECT_NEAR(0, final_tf[1], abs_error<double>::value);
  EXPECT_NEAR(0, final_tf[2], abs_error<double>::value);
  // cell 2
  final_tf = (*cells_r)[1].GetTractorForce();
  EXPECT_NEAR(0, final_tf[0], abs_error<double>::value);
  EXPECT_NEAR(0, final_tf[1], abs_error<double>::value);
  EXPECT_NEAR(0, final_tf[2], abs_error<double>::value);

  // remaining fields should remain unchanged
  // cell 1
  EXPECT_NEAR(0.3, (*cells_r)[0].GetAdherence(), abs_error<double>::value);
  EXPECT_NEAR(9, (*cells_r)[0].GetDiameter(), abs_error<double>::value);
  EXPECT_NEAR(1.4, (*cells_r)[0].GetMass(), abs_error<double>::value);
  // cell 2
  EXPECT_NEAR(0.4, (*cells_r)[1].GetAdherence(), abs_error<double>::value);
  EXPECT_NEAR(11, (*cells_r)[1].GetDiameter(), abs_error<double>::value);
  EXPECT_NEAR(1.1, (*cells_r)[1].GetMass(), abs_error<double>::value);
}

TEST(DisplacementOpTest, ComputeAosoa) {
  std::vector<Cell<Scalar>> cells;
  RunTestDispCell(&cells);
}

TEST(DisplacementOpTest, ComputeSoa) {
  Cell<Soa> cells;
  RunTestDispCell(&cells);
}

}  // namespace bdm
