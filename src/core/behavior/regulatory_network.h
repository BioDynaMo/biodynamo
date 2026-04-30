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

#ifndef CORE_BEHAVIOR_REGULATORY_NETWORK_H_
#define CORE_BEHAVIOR_REGULATORY_NETWORK_H_

#ifdef USE_BOOST

#include "core/behavior/behavior.h"

#ifndef __ROOTCLING__
#include "boost/numeric/odeint.hpp"
#include "boost/phoenix/core.hpp"
#include "boost/phoenix/operator.hpp"
#endif

#ifndef __ROOTCLING__
typedef boost::numeric::ublas::vector<double> b_vector_t;
typedef boost::numeric::ublas::matrix<double> b_matrix_t;
#endif

namespace bdm {

enum class ODE_solver { Euler, Rosenbrock, RungeKutta };

class RegulatoryNetwork : public Behavior {
  BDM_BEHAVIOR_HEADER(RegulatoryNetwork, Behavior, 1);

 public:
  RegulatoryNetwork() { AlwaysCopyToNew(); }
#ifndef __ROOTCLING__
  RegulatoryNetwork(
      real_t dt, int n_dt, const std::vector<real_t>& x, ODE_solver m,
      const std::function<void(const b_vector_t&, b_vector_t&, real_t, Agent*)>&
          rhs,
      const std::function<void(const b_vector_t&, b_matrix_t&, real_t,
                               b_vector_t&, Agent*)>& jacob,
      const std::function<void(const b_vector_t&, real_t, Agent*)>& out) {
    AlwaysCopyToNew();
    SetInitialSpecies(x);
    //
    time_step_ = dt;
    time_subdivision_ = n_dt;
    //
    rhs_ = rhs;
    jacob_ = jacob;
    out_ = out;
    method_ = m;
  }
#endif
  virtual ~RegulatoryNetwork() = default;

  void Initialize(const NewAgentEvent& event) override {
    Base::Initialize(event);

    if (auto* other =
            dynamic_cast<RegulatoryNetwork*>(event.existing_behavior)) {
#ifndef __ROOTCLING__
      current_time_ = other->current_time_;
      current_species_ = other->current_species_;
      previous_species_ = other->previous_species_;
#endif

      time_step_ = other->time_step_;
      time_subdivision_ = other->time_subdivision_;

#ifndef __ROOTCLING__
      rhs_ = other->rhs_;
      jacob_ = other->jacob_;
      out_ = other->out_;
      method_ = other->method_;
#endif
    } else {
      Log::Fatal("RegulatoryNetwork::EventConstructor",
                 "other was not of type RegulatoryNetwork");
    }
  }

#ifndef __ROOTCLING__
  const size_t GetNumberOfSpecies() const { return current_species_.size(); }
  const b_vector_t& GetSpecies() const { return current_species_; }
  const real_t& GetSpecie(size_t i) const { return current_species_[i]; }
#endif

  void Run(Agent* agent) override {
#ifndef __ROOTCLING__
    // update the previous solution
    previous_species_ = current_species_;

    auto ode_rhs_ = [&](const b_vector_t& x, b_vector_t& dxdt, real_t t) {
      rhs_(x, dxdt, t, agent);
    };
    auto ode_jacob_ = [&](const b_vector_t& x, b_matrix_t& jac, real_t t,
                          b_vector_t& dfdt) { jacob_(x, jac, t, dfdt, agent); };

    // initialize the time-integration scheme
    if (ODE_solver::Euler == method_) {
      // define the fixed time increment
      const real_t dt = time_step_ / time_subdivision_;

      // explicit Euler time-integration
      for (int i = 0; i < time_subdivision_; i++) {
        const real_t t = current_time_ + dt * (1 + i);

        // calculate the rate of change of all species
        b_vector_t dxdt(current_species_.size());
        ode_rhs_(current_species_, dxdt, t);

        // update the species
        current_species_ += dxdt * dt;
      }
    } else if (ODE_solver::Rosenbrock == method_) {
      typedef boost::numeric::odeint::rosenbrock4<double> ode_int;

      // set-up the Rosenbrock integrator
      auto stepper =
          boost::numeric::odeint::make_dense_output<ode_int>(1e-6, 1e-6);

      // perform the time-integration
      integrate_const(stepper, std::make_pair(ode_rhs_, ode_jacob_),
                      current_species_, current_time_,
                      (current_time_ + time_step_),
                      (time_step_ / time_subdivision_));
    } else if (ODE_solver::RungeKutta == method_) {
      typedef boost::numeric::odeint::runge_kutta_dopri5<b_vector_t> ode_int;

      // set-up the Runge-Kutta integrator
      auto stepper =
          boost::numeric::odeint::make_dense_output<ode_int>(1e-6, 1e-6);

      // perform the time-integration
      integrate_const(stepper, ode_rhs_, current_species_, current_time_,
                      (current_time_ + time_step_),
                      (time_step_ / time_subdivision_));
    } else {
      Log::Fatal("RegulatoryNetwork::Run",
                 "invalid type of ODE solution method indicated");
    }

    // update the time of the regulatory network
    current_time_ += time_step_;

    // print-out the results
    out_(current_species_, current_time_, agent);
#else
    Log::Fatal("RegulatoryNetwork::Run",
               "this behavior is supported only with \"boost\" installed");
#endif
  };

 protected:
#ifndef __ROOTCLING__
  void SetInitialSpecies(const std::vector<real_t>& x) {
    const size_t n_species = x.size();

    current_species_.resize(n_species);
    previous_species_.resize(n_species);
    for (size_t i = 0; i < n_species; i++)
      current_species_[i] = previous_species_[i] = x[i];
  }
#endif

 private:
  /// Pseudo-time for ODE(s) time integration
  real_t current_time_ = 0.0;
  /// Time-step for ODE(s) time integration
  real_t time_step_ = 1.0;
  int time_subdivision_ = 100;
#ifndef __ROOTCLING__
  /// Current solution of the species concentration
  b_vector_t current_species_ = {};
  /// Previous solution of the species concentration
  b_vector_t previous_species_ = {};
  /// Method used for the ODE(s) numerical solution
  ODE_solver method_;
#endif

#ifndef __ROOTCLING__
  std::function<void(const b_vector_t&, b_vector_t&, real_t, Agent*)> rhs_;
  std::function<void(const b_vector_t&, b_matrix_t&, real_t, b_vector_t&,
                     Agent*)>
      jacob_;
  std::function<void(const b_vector_t&, real_t, Agent*)> out_;
#endif
};

}  // namespace bdm

#endif  // USE_BOOST

#endif  // CORE_BEHAVIOR_REGULATORY_NETWORK_H_
