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

int Odes(real_t t, const real_t y[], real_t f[], void* params) {
  auto* dparams = static_cast<real_t*>(params);
  real_t beta = dparams[0];
  real_t gamma = dparams[1];
  real_t n = dparams[2];

  real_t s = y[0];
  real_t i = y[1];

  // dsdt
  f[0] = -beta * i * s / n;
  // didt
  f[1] = beta * i * s / n - gamma * i;
  // drdt
  f[2] = gamma * i;
  return GSL_SUCCESS;
}

void CalculateAnalyticalSolution(TimeSeries* result, real_t beta, real_t gamma,
                                 real_t susceptible, real_t infected,
                                 real_t tstart, real_t tend, real_t step_size) {
  real_t n = susceptible + infected;
  real_t params[3] = {beta, gamma, n};
  gsl_odeiv2_system sys = {Odes, nullptr, 3, params};

  gsl_odeiv2_driver* d =
      gsl_odeiv2_driver_alloc_y_new(&sys, gsl_odeiv2_step_rk2, 1e-6, 1e-6, 0.0);
  real_t y[3] = {susceptible, infected, 0.0};

  std::vector<real_t> timevec;
  std::vector<real_t> susceptiblevec;
  std::vector<real_t> infectedvec;
  std::vector<real_t> recoveredvec;

  for (real_t t = tstart; t < tend; t += step_size) {
    int status = gsl_odeiv2_driver_apply(d, &tstart, t, y);

    if (status != GSL_SUCCESS) {
      printf("error, return value=%d\n", status);
      break;
    }

    timevec.push_back(t);
    susceptiblevec.push_back(y[0] / n);
    infectedvec.push_back(y[1] / n);
    recoveredvec.push_back(y[2] / n);
  }

  gsl_odeiv2_driver_free(d);

  result->Add("susceptible", timevec, susceptiblevec);
  result->Add("infected", timevec, infectedvec);
  result->Add("recovered", timevec, recoveredvec);
}

}  // namespace bdm
