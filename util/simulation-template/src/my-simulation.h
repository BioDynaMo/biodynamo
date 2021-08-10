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
#ifndef MY_SIMULATION_H_
#define MY_SIMULATION_H_

#include "biodynamo.h"

namespace bdm {

// Example function `SquareMaxTemplate`:
// Computes the square of `to_square` but output is bounded by `upper_bound`.
// This function is not important for the simulation, but introduced to
// illustrate how to test functions occuring in the simulation context.
// See test-suit-util for more info.
double SquareMaxTemplate(double to_square, double upper_bound);

// This function executes the BioDynaMo simulation.
int Simulate(int argc, const char** argv);

}  // namespace bdm

#endif  // MY_SIMULATION_H_
