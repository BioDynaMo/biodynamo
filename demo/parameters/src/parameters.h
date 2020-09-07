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
#ifndef PARAMETERS_H_
#define PARAMETERS_H_

#include "biodynamo.h"

namespace bdm {

// Parameters specific for this simulation
struct SimParam : public ModuleParam {
  BDM_MODULE_PARAM_HEADER(SimParam, 1);

  double foo = 3.14;
  int bar = -42;
};

inline int Simulate(int argc, const char** argv) {
  // Before we create a simulation we have to tell BioDynaMo about
  // the new parameters.
  Param::RegisterModuleParam(new SimParam());

  Simulation simulation(argc, argv);

  // get a pointer to the param object
  auto* param = simulation.GetParam();
  // get a pointer to an instance of SimParam
  auto* sparam = param->GetModuleParam<SimParam>();

  std::cout << "Value of simulation time step " << param->simulation_time_step_
            << std::endl;
  std::cout << "Value of foo                  " << sparam->foo << std::endl;
  std::cout << "Value of bar                  " << sparam->bar << std::endl;

  std::cout << "Simulation completed successfully!" << std::endl;
  return 0;
}

}  // namespace bdm

#endif  // PARAMETERS_H_
