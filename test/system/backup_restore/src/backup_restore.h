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

#ifndef SYSTEM_BACKUP_RESTORE_SRC_BACKUP_RESTORE_H_
#define SYSTEM_BACKUP_RESTORE_SRC_BACKUP_RESTORE_H_

#include <unistd.h>
#include "biodynamo.h"

namespace bdm {

struct TestBehaviour : public BaseBiologyModule {
  TestBehaviour() : BaseBiologyModule(gAllEventIds) {}

  template <typename T>
  void Run(T* cell) {
    usleep(35000);  // 35 ms -> one iteration will take 350 ms
    cell->SetDiameter(cell->GetDiameter() + 1);
  }

  BDM_CLASS_DEF_NV(TestBehaviour, 1);
};

BDM_CTPARAM() {
  BDM_CTPARAM_HEADER();

  BDM_CTPARAM_FOR(bdm, Cell) { using BiologyModules = CTList<TestBehaviour>; };
};

inline int Simulate(int argc, const char** argv) {
  auto set_param = [](auto* param) { param->backup_interval_ = 1; };
  Simulation simulation(argc, argv, set_param);
  auto* rm = simulation.GetResourceManager();

  for (size_t i = 0; i < 10; i++) {
    Cell cell({100.0 * i, 100.0 * i, 100.0 * i});  // no colliding cells
    cell.SetDiameter(i);
    cell.AddBiologyModule(new TestBehaviour());
    rm->push_back(cell);
  }

  auto* scheduler = simulation.GetScheduler();

  // will perform backup after iteration 3
  scheduler->Simulate(3);  // 1050 ms

  // application crash will happen inside this call
  scheduler->Simulate(11);  // 3850 ms

  // another call to Simulate after recovery
  scheduler->Simulate(2);

  // check result
  const auto* cells = rm->Get<Cell>();
  for (size_t i = 0; i < 10; i++) {
    if ((*cells)[i].GetDiameter() != 16 + i) {
      std::cerr << "Test failure: result incorrect" << std::endl;
      std::cerr << "   Diameter of cell " << i << " is "
                << (*cells)[i].GetDiameter() << " but should be 16"
                << std::endl;
      return 1;
    }
  }
  std::cout << "Test finished successfully" << std::endl;
  return 0;
}

}  // namespace bdm

#endif  // SYSTEM_BACKUP_RESTORE_SRC_BACKUP_RESTORE_H_
