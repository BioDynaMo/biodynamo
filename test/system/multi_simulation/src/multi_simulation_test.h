// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & Newcastle University for the benefit of the
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

#ifndef SYSTEM_MULTI_SIMULATION_MULTI_SIMULATION_TEST_H_
#define SYSTEM_MULTI_SIMULATION_MULTI_SIMULATION_TEST_H_

#include <unistd.h>
#include <chrono>
#include <thread>

#include "biodynamo.h"

namespace bdm {
namespace experimental {

using namespace std::chrono_literals;

// Parameters specific for this simulation
struct SimParam : public ParamGroup {
  BDM_PARAM_GROUP_HEADER(SimParam, 1);

  int param1 = 0;
  int param2 = 0;
  int param3 = 0;
};

inline int Simulate(int argc, const char** argv, TimeSeries* result,
                    Param* final_params = nullptr) {
  // Set the optimization parameters
  auto set_param = [&](Param* param) {
    param->Restore(std::move(*final_params));
  };
  Simulation simulation(argc, argv, set_param);

  auto* sparam = simulation.GetParam()->Get<SimParam>();

  // Emulate a simulation
  std::this_thread::sleep_for(100ms);

  result->Add("param1", {0}, {static_cast<double>(sparam->param1)});
  result->Add("param2", {0}, {static_cast<double>(sparam->param2)});
  result->Add("param3", {0}, {static_cast<double>(sparam->param3)});

  std::cout << "Processing parameters: [" << sparam->param1 << ", "
            << sparam->param2 << ", " << sparam->param3 << "]" << std::endl;
  return 0;
}

}  // namespace experimental
}  // namespace bdm

#endif  // SYSTEM_MULTI_SIMULATION_MULTI_SIMULATION_TEST_H_
