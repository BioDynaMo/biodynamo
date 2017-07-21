#ifndef IO_TEST_H_
#define IO_TEST_H_

#include "gtest/gtest.h"

#include "biology_module_util.h"
#include "cell.h"
#include "displacement_op.h"
#include "dividing_cell_op.h"
#include "inline_vector.h"
#include "io_util.h"
#include "test_util.h"
#include "variant.h"

#define ROOTFILE "bdmFile.root"

namespace bdm {

inline void RunInvalidReadTest() {
  auto cells = Cell<>::NewEmptySoa();
  WritePersistentObject(ROOTFILE, "Cells", cells, "RECREATE");

  Cell<Soa>* cells_r = nullptr;

  // Should return 0 if root file doesn't exist
  if (GetPersistentObject("non_existing_file.root", "Cells", cells_r)) {
    FAIL();
  }

  if (!GetPersistentObject(ROOTFILE, "Cells", cells_r)) {
    FAIL();
  }

  remove(ROOTFILE);
}

template <typename T>
void RunTestDivCell(T* cells) {
  cells->push_back(Cell<>(41.0));
  cells->push_back(Cell<>(19.0));

  double volume_mother = (*cells)[0].GetVolume();

  DividingCellOp op;
  op.Compute(cells);

  WritePersistentObject(ROOTFILE, "Cells", *cells, "RECREATE");

  T* cells_r = nullptr;
  GetPersistentObject(ROOTFILE, "Cells", cells_r);

  EXPECT_EQ(3u, cells_r->size());
  EXPECT_NEAR(19.005288996600001, (*cells_r)[1].GetDiameter(),
              abs_error<double>::value);
  EXPECT_NEAR(3594.3640018287319, (*cells_r)[1].GetVolume(),
              abs_error<double>::value);

  // cell got divided so it must be smaller than before
  // more detailed division test can be found in `cell_test.h`
  EXPECT_GT(41, (*cells_r)[0].GetDiameter());
  EXPECT_GT(41, (*cells_r)[2].GetDiameter());
  // volume of two daughter cells must be equal to volume of the mother
  EXPECT_NEAR(volume_mother,
              (*cells_r)[0].GetVolume() + (*cells_r)[2].GetVolume(),
              abs_error<double>::value);

  remove(ROOTFILE);
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

  T* cells_r = nullptr;
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

  remove(ROOTFILE);
}

}  // namespace bdm

#endif  // IO_TEST_H_
