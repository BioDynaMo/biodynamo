// -----------------------------------------------------------------------------
//
// Copyright (C) The BioDynaMo Project.
// All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#include "core/biology_module/regulate_genes.h"
#include "core/param/param.h"
#include "core/simulation.h"
#include "core/scheduler.h"

namespace bdm {

  RegulateGenes::RegulateGenes() : BaseBiologyModule(gAllEventIds) {}

  RegulateGenes::RegulateGenes(EventId event) : BaseBiologyModule(event) {}

  RegulateGenes::RegulateGenes(const Event& event, BaseBiologyModule* other, uint64_t new_oid) : BaseBiologyModule(event, other, new_oid) {
    if(RegulateGenes* rgbm = dynamic_cast<RegulateGenes*>(other)) {
      concentrations_ = rgbm->concentrations_;
      first_derivatives_ = rgbm->first_derivatives_;
    } else {
      Log::Fatal("RegulateGenes::EventConstructor", "other was not of type RegulateGenes");
    }
  }

  RegulateGenes::~RegulateGenes() {}

  /// Create a new instance of this object using the default constructor.
  BaseBiologyModule* RegulateGenes::GetInstance(const Event& event, BaseBiologyModule* other, uint64_t new_oid) const {
    return new RegulateGenes(event, other, new_oid);
  }

  /// Create a copy of this biology module.
  BaseBiologyModule* RegulateGenes::GetCopy() const { return new RegulateGenes(*this); }

  /// Empty default event handler.
  void RegulateGenes::EventHandler(const Event &event, BaseBiologyModule *other1, BaseBiologyModule* other2) {
    BaseBiologyModule::EventHandler(event, other1, other2);
  }

  /// AddGene adds a new differential equation.
  /// \param first_derivative differential equation in the form:
  ///        `slope = f(time, last_concentration)` -- e.g.:
  ///
  ///              [](double time, double last_concentration) {
  ///                 return 1 - time * last_concentration;
  ///              }
  ///
  /// \param initial_concentration
  void RegulateGenes::AddGene(std::function<double(double, double)> first_derivative,
               double initial_concentration) {
    first_derivatives_.push_back(first_derivative);
    concentrations_.push_back(initial_concentration);
  }

  const std::vector<double>& RegulateGenes::GetConcentrations() const {
    return concentrations_;
  }

  /// Method Run() contains the implementation for Runge-Khutta and Euler
  /// methods for solving ODE.
  void RegulateGenes::Run(SimObject* sim_object) {
    auto* sim = Simulation::GetActive();
    auto* param = sim->GetParam();
    auto* scheduler = sim->GetScheduler();

    const auto& timestep = param->simulation_time_step_;
    uint64_t simulated_steps = scheduler->GetSimulatedSteps();
    const auto absolute_time = simulated_steps * timestep;

    if (param->numerical_ode_solver_ == Param::NumericalODESolver::kEuler) {
      // Euler
      for (uint64_t i = 0; i < first_derivatives_.size(); i++) {
        double slope = first_derivatives_[i](absolute_time, concentrations_[i]);
        concentrations_[i] += slope * timestep;
      }
    } else if (param->numerical_ode_solver_ ==
               Param::NumericalODESolver::kRK4) {
      // Runge-Kutta 4
      for (uint64_t i = 0; i < first_derivatives_.size(); i++) {
        double interval_midpoint = absolute_time + timestep / 2.0;
        double interval_endpoint = absolute_time + timestep;

        double k1 = first_derivatives_[i](absolute_time, concentrations_[i]);
        double k2 = first_derivatives_[i](
            interval_midpoint, concentrations_[i] + timestep * k1 / 2.0);
        double k3 = first_derivatives_[i](
            interval_midpoint, concentrations_[i] + timestep * k2 / 2.0);
        double k4 = first_derivatives_[i](interval_endpoint,
                                          concentrations_[i] + timestep * k3);

        concentrations_[i] += timestep / 6.0 * (k1 + 2 * k2 + 2 * k3 + k4);
      }
    }
  }

}  // namespace bdm
