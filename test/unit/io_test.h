#ifndef UNIT_IO_TEST_H_
#define UNIT_IO_TEST_H_

#include <gtest/gtest.h>
#include "io_util.h"
#include "param.h"

namespace bdm {

/// Test fixture for io tests that follow the same form
/// Usage:
///
///     TEST_F(IOTest, Type) {
///       // assign value to each data member
///       Type t;
///       t.SetDataMember1(...);
///       ...
///       Type *restored = nullptr;
///
///       BackupAndRestore(t, &restored);
///
///       // verify if all data members have been restored correctly
///       EXPECT_EQ(..., restored->GetDataMember1());
///       ...
///     }
class IOTest : public ::testing::Test {
 public:
  static constexpr char const* kRootFile = "io-test.root";

 protected:
  virtual void SetUp() {
    Param::Reset();
    remove(kRootFile);
  }

  virtual void TearDown() {
    Param::Reset();
    // remove(kRootFile);
  }
};

/// Writes backup to file and reads it back into restored
/// Outside the test fixture so it can be called in a function from the header.
/// TEST_F can't be added a header due to multiple references linking error
/// and must be placed in a source file.
template <typename T>
void BackupAndRestore(const T& backup, T** restored) {
  // write to root file
  WritePersistentObject(IOTest::kRootFile, "T", backup, "new");

  // read back
  GetPersistentObject(IOTest::kRootFile, "T", *restored);
}

}  // namespace bdm

#endif  // UNIT_IO_TEST_H_
