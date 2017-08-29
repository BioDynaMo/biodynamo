#ifndef UNIT_IO_UTIL_TEST_H_
#define UNIT_IO_UTIL_TEST_H_

#include "gtest/gtest.h"

#include "biology_module_util.h"
#include "cell.h"
#include "displacement_op.h"
#include "dividing_cell_op.h"
#include "inline_vector.h"
#include "io_util.h"
#include "unit/test_util.h"
#include "variant.h"

#define ROOTFILE "bdmFile.root"

namespace bdm {

inline void RunInvalidReadTest() {
  auto cells = Cell::NewEmptySoa();
  WritePersistentObject(ROOTFILE, "Cells", cells, "RECREATE");

  SoaCell* cells_r = nullptr;

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
  cells->push_back(Cell(41.0));
  cells->push_back(Cell(19.0));

  double volume_mother = (*cells)[0].GetVolume();

  DividingCellOp op;
  op(cells, 0);

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

}  // namespace bdm

#endif  // UNIT_IO_UTIL_TEST_H_
