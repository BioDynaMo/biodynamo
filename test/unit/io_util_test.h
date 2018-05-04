#ifndef UNIT_IO_UTIL_TEST_H_
#define UNIT_IO_UTIL_TEST_H_

#include "gtest/gtest.h"

#include "biology_module_util.h"
#include "cell.h"
#include "displacement_op.h"
#include "dividing_cell_op.h"
#include "inline_vector.h"
#include "io_util.h"
#include "unit/default_ctparam.h"
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

}  // namespace bdm

#endif  // UNIT_IO_UTIL_TEST_H_
