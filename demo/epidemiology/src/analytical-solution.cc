// -----------------------------------------------------------------------------
//
// Copyright (C) Lukas Breitwieser.
// All Rights Reserved.
//
// -----------------------------------------------------------------------------

#include "analytical-solution.h"
#include <gsl/gsl_errno.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_odeiv2.h>

namespace bdm {

int Odes(double t, const double y[], double f[], void* params) {
  auto* dparams = static_cast<double*>(params);
  double beta = dparams[0];
  double gamma = dparams[1];
  double n = dparams[2];

  double s = y[0];
  double i = y[1];

  // dsdt
  f[0] = -beta * i * s / n;
  // didt
  f[1] = beta * i * s / n - gamma * i;
  // drdt
  f[2] = gamma * i;
  return GSL_SUCCESS;
}

void CalculateAnalyticalSolution(ResultData* result, double beta, double gamma,
                                 double susceptible, double infected,
                                 double tstart, double tend, double step_size) {
  double n = susceptible + infected;
  double params[3] = {beta, gamma, n};
  gsl_odeiv2_system sys = {Odes, nullptr, 3, params};

  gsl_odeiv2_driver* d =
      gsl_odeiv2_driver_alloc_y_new(&sys, gsl_odeiv2_step_rk2, 1e-6, 1e-6, 0.0);
  double y[3] = {susceptible, infected, 0.0};

  auto num_steps = (tend - tstart) / step_size;
  result->time_.reserve(num_steps);
  result->susceptible_.reserve(num_steps);
  result->infected_.reserve(num_steps);
  result->recovered_.reserve(num_steps);

  for (double t = tstart; t < tend; t += step_size) {
    int status = gsl_odeiv2_driver_apply(d, &tstart, t, y);

    if (status != GSL_SUCCESS) {
      printf("error, return value=%d\n", status);
      break;
    }

    result->time_.push_back(t);
    result->susceptible_.push_back(y[0] / n);
    result->infected_.push_back(y[1] / n);
    result->recovered_.push_back(y[2] / n);
  }

  gsl_odeiv2_driver_free(d);
}

}  // namespace bdm
