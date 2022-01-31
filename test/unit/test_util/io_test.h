// -----------------------------------------------------------------------------
//
// Copyright (C) 2022 CERN & University of Surrey for the benefit of the
// BioDynaMo collaboration. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#ifndef UNIT_TEST_UTIL_IO_TEST_H_
#define UNIT_TEST_UTIL_IO_TEST_H_

#include <gtest/gtest.h>
#include "core/param/param.h"
#include "core/simulation.h"
#include "core/util/io.h"
#include "unit/test_util/test_util.h"

#include "TBufferJSON.h"

#define ROOT_FILE "io-test.root"
#define JSON_FILE "io-test.json"

namespace bdm {

// Hide testing::Test from the generated dictionary.
// Otherwise ROOT is unable to load it into cling.
#ifndef __ROOTCLING__

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
class IOTest : public ::testing::Test {};

#endif  // __ROOTCLING__

/// Writes backup to file and reads it back into restored
/// Outside the test fixture so it can be called in a function from the header.
/// TEST_F can't be used inside a header due to multiple references linking
/// error and must be placed in a source file.
template <typename T>
void BackupAndRestore(const T& backup, T** restored) {
  remove(ROOT_FILE);
  remove(JSON_FILE);

  // write to root file
  WritePersistentObject(ROOT_FILE, "T", backup, "new");

  // Two benefits of writing an object to JSON:
  // 1) Ensures that a dictionary must exists; otherwise linking error
  // 2) Print out the object's content for debugging purposes
  TBufferJSON::ExportToFile(JSON_FILE, &backup, backup.Class());

  // read back
  GetPersistentObject(ROOT_FILE, "T", *restored);
}

}  // namespace bdm

#endif  // UNIT_TEST_UTIL_IO_TEST_H_
