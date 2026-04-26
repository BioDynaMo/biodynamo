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

#ifndef OSCILLATOR_H_
#define OSCILLATOR_H_

#include <fstream>
#include <iostream>

#ifndef __ROOTCLING__
#include <boost/array.hpp>
#include "boost/numeric/odeint.hpp"
#include "boost/phoenix/core.hpp"
#include "boost/phoenix/operator.hpp"
#endif

#ifndef __ROOTCLING__
typedef boost::numeric::ublas::vector<double> b_vector_t;
typedef boost::numeric::ublas::matrix<double> b_matrix_t;
#endif

namespace oscillator {

#ifndef __ROOTCLING__
struct ODE_system {
  void operator()(const b_vector_t& x, b_vector_t& dxdt, double t) const {
    dxdt[0] = x[1];
    dxdt[1] = mu * x[1] - mu * x[0] * x[0] * x[1] - x[0];
  }
  // https://en.wikipedia.org/wiki/Van_der_Pol_oscillator
  double mu = 100.0;
};

struct ODE_jacobian {
  void operator()(const b_vector_t& x, b_matrix_t& jac, double t,
                  b_vector_t& dfdt) const {
    jac(0, 0) = 0.0;
    jac(0, 1) = 1.0;
    jac(1, 0) = -2.0 * mu * x[0] * x[1] - 1.0;
    jac(1, 1) = mu - mu * x[0] * x[0];
    //
    dfdt[0] = dfdt[1] = 0.0;
  }
  // https://en.wikipedia.org/wiki/Van_der_Pol_oscillator
  double mu = 100.0;
};

struct ODE_output {
  void operator()(const b_vector_t& x, double t) {
    std::clog << t << ',' << x[0] << ',' << x[1] << std::endl;
  }
};
#endif

inline int Simulate(int argc, const char** argv) {
  std::ofstream fout("oscillator.csv");
  // save the original buffer of std::clog
  std::streambuf* orig_clog_buff = std::clog.rdbuf();
  // redirect std::clog to point to the above file
  std::clog.rdbuf(fout.rdbuf());

#ifndef __ROOTCLING__
  b_vector_t xy(2);
  xy[0] = 2;
  xy[1] = 0;

  typedef boost::numeric::odeint::rosenbrock4<double> ode_int;

  // set-up the Rosenbrock integrator
  auto stepper = boost::numeric::odeint::make_dense_output<ode_int>(1e-6, 1e-6);

  // perform the time-integration
  integrate_const(stepper, std::make_pair(ODE_system(), ODE_jacobian()), xy,
                  0.0, 500.0, 1.0e-2, ODE_output());
#endif

  // restore the original buffer of std::clog
  std::clog.rdbuf(orig_clog_buff);

  return 0;
}

}  // namespace oscillator

#endif  // OSCILLATOR_H_
