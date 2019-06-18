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

#ifndef CORE_BIOLOGY_MODULE_REGULATE_GENES_H_
#define CORE_BIOLOGY_MODULE_REGULATE_GENES_H_

#include <functional>
#include <vector>

#include "core/biology_module/biology_module.h"
#include "core/param/param.h"
#include "core/scheduler.h"
#include "core/simulation.h"
#include "core/util/root.h"

namespace bdm {

/// This module simulates expression of genes and contains all required
/// additional variables for tracking of the concentration of proteins.
/// Thus, it can work with any type of simulation object.
/// It has the implementation of Euler and Runge-Kutta numerical methods
/// for solving ODE. Both methods implemented inside the body of method Run().
/// The user determines which method is picked in particular simulation
/// through variable `Param::numerical_ode_solver_`.
struct RegulateGenes : public BaseBiologyModule {
  RegulateGenes() : BaseBiologyModule(gAllEventIds) {}

  explicit RegulateGenes(EventId event) : BaseBiologyModule(event) {}

  RegulateGenes(const Event& event, BaseBiologyModule* other,
                uint64_t new_oid = 0)
      : BaseBiologyModule(event, other, new_oid) {
    if (RegulateGenes* rgbm = dynamic_cast<RegulateGenes*>(other)) {
      concentrations_ = rgbm->concentrations_;
      first_derivatives_ = rgbm->first_derivatives_;
    } else {
      Log::Fatal("RegulateGenes::EventConstructor",
                 "other was not of type RegulateGenes");
    }
  }

  /// Create a new instance of this object using the default constructor.
  BaseBiologyModule* GetInstance(const Event& event, BaseBiologyModule* other,
                                 uint64_t new_oid = 0) const override {
    return new RegulateGenes(event, other, new_oid);
  }

  /// Create a copy of this biology module.
  BaseBiologyModule* GetCopy() const override {
    return new RegulateGenes(*this);
  }

  /// Empty default event handler.
  void EventHandler(const Event& event, BaseBiologyModule* other1,
                    BaseBiologyModule* other2 = nullptr) override {
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
  void AddGene(std::function<double(double, double)> first_derivative,
               double initial_concentration) {
    first_derivatives_.push_back(first_derivative);
    concentrations_.push_back(initial_concentration);
  }

  const std::vector<double>& GetConcentrations() const {
    return concentrations_;
  }

  /// Method Run() contains the implementation for Runge-Khutta and Euler
  /// methods for solving ODE.
  void Run(SimObject* sim_object) override {
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

 private:
  /// Store the current concentration for each gene
  std::vector<double> concentrations_ = {};

  /// Store the gene differential equations, which define how the concentration
  /// change.
  /// New functions can be added through method AddGene()
  std::vector<std::function<double(double, double)>> first_derivatives_ = {};

  BDM_CLASS_DEF_OVERRIDE(RegulateGenes, 1);
};

}  // namespace bdm

#endif  // CORE_BIOLOGY_MODULE_REGULATE_GENES_H_
