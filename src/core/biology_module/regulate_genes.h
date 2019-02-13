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
#include "core/util/root.h"

namespace bdm {

/// This module simulates expression of genes and contains all required
/// additional variables for tracking of the concentration of proteins.
/// Thus, can work with any type of simulation object.
/// It has the implementation of Euler and Runge-Kutta numerical methods
/// for solving ODE. Both methods implemented inside the body of method Run().
///  The user determines which method is picked in particular simulation
/// through variable `Param::numerical_ode_solver_`.
struct RegulateGenes : public BaseBiologyModule {
  RegulateGenes();

  explicit RegulateGenes(EventId event);

  RegulateGenes(const Event& event, BaseBiologyModule* other,
                uint64_t new_oid = 0);

  virtual ~RegulateGenes();

  /// Create a new instance of this object using the default constructor.
  BaseBiologyModule* GetInstance(const Event& event, BaseBiologyModule* other,
                                 uint64_t new_oid = 0) const override;

  /// Create a copy of this biology module.
  BaseBiologyModule* GetCopy() const override;

  /// Empty default event handler.
  void EventHandler(const Event& event, BaseBiologyModule* other1,
                    BaseBiologyModule* other2 = nullptr) override;

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
               double initial_concentration);

  const std::vector<double>& GetConcentrations() const;

  /// Method Run() contains the implementation for Runge-Khutta and Euler
  /// methods for solving ODE.
  void Run(SimObject* sim_object) override;

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
