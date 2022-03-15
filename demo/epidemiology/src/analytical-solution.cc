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

int Odes(real t, const real y[], real f[], void* params) {
  auto* dparams = static_cast<real*>(params);
  real beta = dparams[0];
  real gamma = dparams[1];
  real n = dparams[2];

  real s = y[0];
  real i = y[1];

  // dsdt
  f[0] = -beta * i * s / n;
  // didt
  f[1] = beta * i * s / n - gamma * i;
  // drdt
  f[2] = gamma * i;
  return GSL_SUCCESS;
}

void CalculateAnalyticalSolution(TimeSeries* result, real beta, real gamma,
                                 real susceptible, real infected,
                                 real tstart, real tend, real step_size) {
  real n = susceptible + infected;
  real params[3] = {beta, gamma, n};
  gsl_odeiv2_system sys = {Odes, nullptr, 3, params};

  gsl_odeiv2_driver* d =
      gsl_odeiv2_driver_alloc_y_new(&sys, gsl_odeiv2_step_rk2, 1e-6, 1e-6, 0.0);
  real y[3] = {susceptible, infected, 0.0};

  std::vector<real> timevec;
  std::vector<real> susceptiblevec;
  std::vector<real> infectedvec;
  std::vector<real> recoveredvec;

  for (real t = tstart; t < tend; t += step_size) {
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
