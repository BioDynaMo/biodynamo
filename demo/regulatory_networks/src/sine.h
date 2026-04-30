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

#ifndef SINE_H_
#define SINE_H_

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

namespace sine {

#ifndef __ROOTCLING__
struct ODE_system {
  void operator()(const b_vector_t& x, b_vector_t& dxdt, double t) const {
    dxdt[0] = A * cos(t);
  }
  //
  const double A = 10.0;
};

struct ODE_output {
  void operator()(const b_vector_t& x, double t) {
    std::clog << t << ',' << x[0] << std::endl;
  }
};
#endif

inline int Simulate(int argc, const char** argv) {
  std::ofstream fout("sine.csv");
  // save the original buffer of std::clog
  std::streambuf* orig_clog_buff = std::clog.rdbuf();
  // redirect std::clog to point to the above file
  std::clog.rdbuf(fout.rdbuf());

#ifndef __ROOTCLING__
  b_vector_t x(1);
  x[0] = 0.0;

  typedef boost::numeric::odeint::runge_kutta_dopri5<b_vector_t> ode_int;

  // set-up the Runge-Kutta integrator
  auto stepper = boost::numeric::odeint::make_dense_output<ode_int>(1e-6, 1e-6);

  // perform the time-integration
  integrate_const(stepper, ODE_system(), x, 0.0, 12.5663706144, 0.001,
                  ODE_output());
#endif

  // restore the original buffer of std::clog
  std::clog.rdbuf(orig_clog_buff);

  return 0;
}

}  // namespace sine

#endif  // SINE_H_
