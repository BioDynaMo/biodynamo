// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & University of Surrey for the benefit of the
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

#ifndef SYSTEM_BACKUP_RESTORE_SRC_BACKUP_RESTORE_H_
#define SYSTEM_BACKUP_RESTORE_SRC_BACKUP_RESTORE_H_

#include <unistd.h>
#include "biodynamo.h"

namespace bdm {

struct TestBehaviour : public Behavior {
  BDM_BEHAVIOR_HEADER(TestBehaviour, Behavior, 1);

  TestBehaviour() { AlwaysCopyToNew(); }

  void Run(Agent* agent) override {
    usleep(35000);  // 35 ms -> one iteration will take 350 ms
    agent->SetDiameter(agent->GetDiameter() + 1);
  }
};

inline int Simulate(int argc, const char** argv) {
  auto set_param = [](Param* param) { param->backup_interval = 1; };
  Simulation simulation(argc, argv, set_param);
  auto* rm = simulation.GetResourceManager();

  for (size_t i = 0; i < 10; i++) {
    auto* cell = new Cell({real_t(100.0) * i, real_t(100.0) * i,
                           real_t(100.0) * i});  // no colliding cells
    cell->SetDiameter(i);
    cell->AddBehavior(new TestBehaviour());
    rm->AddAgent(cell);
  }

  auto* scheduler = simulation.GetScheduler();

  // will perform backup after iteration 3
  scheduler->Simulate(3);  // 1050 ms

  // application crash will happen inside this call
  scheduler->Simulate(11);  // 3850 ms

  // another call to Simulate after recovery
  scheduler->Simulate(2);

  // check result
  int count = 0;
  bool failed = 0;
  rm->ForEachAgent([&](Agent* agent) {
    if (agent->GetDiameter() != 16 + count) {
      std::cerr << "Test failure: result incorrect" << std::endl;
      std::cerr << "   Diameter of cell " << count << " is "
                << agent->GetDiameter() << " but should be 16" << std::endl;
      failed = true;
      return;
    }
    count++;
  });
  if (failed) {
    return 1;
  }
  std::cout << "Test finished successfully" << std::endl;
  return 0;
}

}  // namespace bdm

#endif  // SYSTEM_BACKUP_RESTORE_SRC_BACKUP_RESTORE_H_
