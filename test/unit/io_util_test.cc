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

#include "unit/io_util_test.h"
#include "simulation_implementation.h"

namespace bdm {

TEST(IOUtilTest, InvalidRead) { RunInvalidReadTest(); }

TEST(IOUtilTest, RuntimeVars) {
  RuntimeVariables this_machine;

  SysInfo_t si = this_machine.GetSystemInfo();
  si.fOS = "Non-Existing_OS";
  RuntimeVariables different_machine;
  different_machine.SetSystemInfo(si);

  if (this_machine == different_machine) {
    FAIL();
  }

  RuntimeVariables this_machine_copy;
  if (this_machine != this_machine_copy) {
    FAIL();
  }

  WritePersistentObject(ROOTFILE, "RuntimeVars", this_machine, "RECREATE");
  RuntimeVariables* this_machine_r = nullptr;
  GetPersistentObject(ROOTFILE, "RuntimeVars", this_machine_r);

  if (this_machine != *this_machine_r) {
    FAIL();
  }

  remove(ROOTFILE);
}

}  // namespace bdm
