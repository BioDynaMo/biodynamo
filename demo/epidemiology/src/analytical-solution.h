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

void CalculateAnalyticalSolution(ResultData* result, double beta, double gamma,
                                 double susceptible, double infected,
                                 double tstart, double tend, double step_size);

}  // namespace bdm

#endif  // ANALYTICAL_SOLUTION_H_
