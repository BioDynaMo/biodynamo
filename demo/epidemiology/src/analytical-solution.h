// -----------------------------------------------------------------------------
//
// Copyright (C) Lukas Breitwieser.
// All Rights Reserved.
//
// -----------------------------------------------------------------------------

#ifndef ANALYTICAL_SOLUTION_H_
#define ANALYTICAL_SOLUTION_H_

#include "evaluate.h"

namespace bdm {

void CalculateAnalyticalSolution(TimeSeries* result, real beta, real gamma,
                                 real susceptible, real infected,
                                 real tstart, real tend, real step_size);

}  // namespace bdm

#endif  // ANALYTICAL_SOLUTION_H_
