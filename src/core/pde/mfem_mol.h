// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & Newcastle University for the benefit of the
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

#ifdef USE_MFEM
#ifndef MFEM_MOL_H_
#define MFEM_MOL_H_

#include <vector>
#include "core/pde/timedependent_operators/conduction_operator.h"
#include "core/pde/timedependent_operators/diffusion_operator.h"
#include "mfem.hpp"

namespace bdm {
namespace experimental {

enum MFEMODESolver {
  kBackwardEulerSolver,
  kSDIRK23Solver2,
  kSDIRK33Solver,
  kForwardEulerSolver,
  kRK2Solver,
  kRK3SSPSolver,
  kRK4Solver,
  kGeneralizedAlphaSolver,
  kImplicitMidpointSolver,
  kSDIRK23Solver1,
  kSDIRK34Solver
};

enum PDEOperator { kDiffusion, kDiffusionWithFunction, kConduction };

class MethodOfLineSolver {
 protected:
  /// Arbitrary order H1-conforming (continuous) finite elements. (quote MFEM)
  mfem::H1_FECollection fe_coll_;
  /// The underlying mesh on which we solve the PDE
  mfem::Mesh* mesh_;
  /// Class FiniteElementSpace - responsible for providing FEM view of the
  /// mesh, mainly managing the set of degrees of freedom. (quote MFEM)
  mfem::FiniteElementSpace fespace_;
  /// The solution computed from the PDE.
  mfem::GridFunction u_gf_;
  /// The ODE solver used to integrate in time.
  mfem::ODESolver* ode_solver_;
  /// Vector representation of the PDE solution.
  mfem::Vector u_;
  /// Operator descibing the FE discretization of the MOL method.
  MolOperator* operator_;
  /// Function to initialize Grid values
  std::function<double(const mfem::Vector&)> InitialGridValues_;
  /// Vector for numeric constants that we feed to the constructor of the
  /// operators.
  std::vector<double> numeric_operator_parameters_;
  /// Vector for functions that we feed to the constructor of the
  /// operators.
  std::vector<std::function<double(const mfem::Vector&)>> operator_functions_;
  /// Value to store the current time
  double t_;

  /// Internally used function for initilization.
  void Initialize();
  /// Function used to set the boundaries conditions.
  void SetBoundaryConditions();

 public:
  MethodOfLineSolver(
      mfem::Mesh* mesh, int order, int dimension, int ode_solver_id,
      int pde_oper_id,
      std::function<double(const mfem::Vector&)> InitialGridValues,
      std::vector<double> numeric_operator_parameters,
      std::vector<std::function<double(const mfem::Vector&)>>
          operator_functions);
  ~MethodOfLineSolver();
  // No copy (assignment) as of now.
  MethodOfLineSolver(const MethodOfLineSolver&) = delete;
  MethodOfLineSolver& operator=(const MethodOfLineSolver&) = delete;
  MethodOfLineSolver(MethodOfLineSolver&&) = delete;
  MethodOfLineSolver& operator=(MethodOfLineSolver&&) = delete;

  /// Function to set the ODESolver. See enum MFEMODESolver for options.
  void SetODESolver(int solver_id);
  /// Set the operator, e.g. define the PDE to solve.
  void SetOperator(int operator_id);

  /// Execute one ODE timestep `dt`, e.g. compute `u(t+dt)` from `u(t)`.
  void Step(double dt);

  /// Export the continuum model to paraview.
  void Visualize();

  /// Update the grid function. The ODE procedure operates on the coefficient
  /// vector `u_` and updates it. Before making calls to `u_gf_`, this routine
  /// must be called to update it.
  void UpdateGridFunction();

  /// Get the value of the GridFunction solution at a certain position.
  double GetSolutionAtPosition();

  /// Set the operator for MOL, e.g. define the equation.
  void SetOperator(MolOperator* oper);

  /// Get reference to fespace_, needed for custom operator
  mfem::FiniteElementSpace& GetFESpace() { return fespace_; }

  // Get the ODESolver associated to the MOL solver
  mfem::ODESolver* GetODESolver() { return ode_solver_; }

  // Get the MOLOperator associated to the MOL solver
  MolOperator* GetMolOperator() { return operator_; }

  // Get simulated time
  double GetSimTime() { return t_; }
};

}  // namespace experimental
}  // namespace bdm

#endif  // MFEM_MOL_H_
#endif  // USE_MFEM
