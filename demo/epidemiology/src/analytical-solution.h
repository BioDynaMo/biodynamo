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

void CalculateAnalyticalSolution(TimeSeries* result, real_t beta, real_t gamma,
                                 real_t susceptible, real_t infected,
                                 real_t tstart, real_t tend, real_t step_size);

}  // namespace bdm

#endif  // ANALYTICAL_SOLUTION_H_
