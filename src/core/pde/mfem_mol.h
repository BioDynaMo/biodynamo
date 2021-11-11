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

// ToDo(tobias):
// * add agent properties or environment properties such that we
//   have a quick mapping from agent to mesh element / integration point. That
//   we only update if necessary.
// * Avoid update for pure diffusion for performance reasons.

#ifdef USE_MFEM
#ifndef MFEM_MOL_H_
#define MFEM_MOL_H_

#include <limits.h>
#include <vector>
#include "core/agent/agent.h"
#include "core/container/math_array.h"
#include "core/pde/timedependent_operators/conduction_operator.h"
#include "core/pde/timedependent_operators/diffusion_operator.h"
#include "mfem.hpp"

namespace bdm {
namespace experimental {

/// Enum to specify the ODE solver for the TimeDependentScalarField3d. For a
/// detailed explanation of the different solvers, please consult the `MFEM`
/// documentation.
/// API: http://mfem.github.io/doxygen/html/classmfem_1_1ODESolver.html
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

/// Enum to specify the type of PDE for the TimeDependentScalarField3d.
/// kDiffusion:
/// \f[ \frac{du}{dt} = \nabla (D \nabla u), \ \f]
/// kDiffusionWithFunction:
/// \f[ \frac{du}{dt} = \nabla (D \nabla u) + \Gamma u, \ \f]
/// kConduction:
/// \f[ \frac{du}{dt} = \nabla \cdot (\kappa + \alpha u) \nabla u \f]
enum PDEOperator { kDiffusion, kDiffusionWithFunction, kConduction };

/// Converts a bdm::Double3 to a mfem::Vector.
mfem::Vector ConvertToMFEMVector(const Double3& position);

/// This class models a time dependent scalar field \f$ \phi \f$ in real, three
/// dimensional space \f$ \mathbb{R}^3 \f$, i.e. \f$ \phi (x,t): \mathbb{R}^3
/// \times \mathbb{R} \rightarrow \mathbb{R}\f$. The user must describe the time
/// dependent scalar field with a partial differential equation. Technically,
/// this is done by specifying a suitable (spatial) PDEOperator. This class
/// represents a modular interface to the `MFEM` finite element library and is
/// conceptually implementing the method of lines. We use a finite element
/// discretization in space to derive a high dimensional ODE system, which we
/// integrate over time with established numerical methods (see MFEMODESolver).
/// For more details regarding the method of lines, one may consult the
/// following sources:
///  - <a
///    href="http://www.scholarpedia.org/article/Method_of_lines">Scholarpedia</a>
///  - <a href="https://en.wikipedia.org/wiki/Method_of_lines">Wikipedia</a>
///  - <a
///    href="https://github.com/mfem/mfem/blob/master/examples/ex16.cpp">MFEM
///    Examples</a>
class TimeDependentScalarField3d {
 protected:
  /// Arbitrary order H1-conforming (continuous) finite elements. (quote MFEM)
  mfem::H1_FECollection fe_coll_;
  /// The underlying mesh on which we solve the PDE
  mfem::Mesh* mesh_;
  /// Vertex to Element Table from mfem mesh. This is likely expensive to
  /// construct and therefore saved during the first call to optimize
  /// performance.
  mfem::Table* table_of_elements_;
  /// Class FiniteElementSpace - responsible for providing FEM view of the
  /// mesh, mainly managing the set of degrees of freedom. (quote MFEM)
  mfem::FiniteElementSpace fespace_;
  /// The solution computed from the PDE.
  mfem::GridFunction u_gf_;
  /// The ODE solver used to integrate in time.
  mfem::ODESolver* ode_solver_;
  /// Vector representation of the PDE solution, i.e. the actual degrees of
  /// freedom or
  mfem::Vector u_;
  /// Operator describing the FE discretization of the MOL method.
  MolOperator* operator_;
  /// Function to initialize Grid values
  std::function<double(const mfem::Vector&)> InitialGridValues_;
  /// Vector for numeric constants that we feed to the constructor of the
  /// operators. See definition of SetOperator(int) to see where the constants
  /// end up.
  std::vector<double> numeric_operator_parameters_;
  /// Vector for functions that we feed to the constructor of the
  /// operators. See definition of SetOperator(int) to see where the functions
  /// end up.
  std::vector<std::function<double(const mfem::Vector&)>> operator_functions_;
  /// Value to store the current time / simulated time.
  double t_;
  /// ID of substance / continuum variable considered by an object instance.
  uint64_t substance_id_;
  /// ID of substance / continuum variable considered by an object instance.
  std::string substance_name_;
  /// Number of calls to Step()
  uint64_t ode_steps_;

  /// Internally used function for initialization of the ODE system.
  void Initialize();
  /// Function used to set the boundaries conditions.
  void SetBoundaryConditions();

  /// Wrapper to mfem::Mesh::FindPoints(). First iterates over all elements
  /// to find the closest center. Afterwards, it checks if the position is
  /// either in the respective element or in it's neighbouring elements. This
  /// member function is protected since it is expensive to call and should
  /// therefore be used with caution. To obtain the value of the solution at a
  /// specific point, please use the GetSolutionAtPosition. It also returns the
  /// element belonging to that specific location.
  std::pair<int, mfem::IntegrationPoint> FindPointInMesh(
      const Double3& position);

  // ToDo(tobias): remove from api once benchmark script has changed.
  /// Given an element_id and a matching integration point, this function
  /// returns the solution value.
  double GetSolutionInElementAndIntegrationPoint(
      int element_id, const mfem::IntegrationPoint& integration_point);

 public:
  /// Implementation of the Method of Lines based on MFEM.
  ///
  /// @param[in]  mesh             The mesh discretization of the domain (MEFM)
  /// @param[in]  substance_id     The substance identifier
  /// @param[in]  substance_name   The substance name
  /// @param[in]  order            Polynomial order for FE method
  /// @param[in]  dimension        Dimension of the Problem (only `3` supported)
  /// @param[in]  ode_solver_id    ID to specify the ODE solver
  /// @param[in]  pde_oper_id      Specify the operator / the PDE problem
  /// @param[in]  InitialGridValues Function to set the initial conditions
  /// @param[in]  numeric_operator_parameters  Numeric constants for PDE problem
  /// @param[in]  operator_functions Functions occuring in the PDE
  ///
  /// For more details please take a look at the implemenation.
  TimeDependentScalarField3d(
      mfem::Mesh* mesh, int order, int dimension, MFEMODESolver ode_solver_id,
      PDEOperator pde_oper_id,
      std::function<double(const mfem::Vector&)> InitialGridValues,
      std::vector<double> numeric_operator_parameters,
      std::vector<std::function<double(const mfem::Vector&)>>
          operator_functions);
  /// Destructor calls delete for the ODE solver and the operator.
  ~TimeDependentScalarField3d();
  // No copy (assignment) as of now.
  TimeDependentScalarField3d(const TimeDependentScalarField3d&) = delete;
  TimeDependentScalarField3d& operator=(const TimeDependentScalarField3d&) =
      delete;
  TimeDependentScalarField3d(TimeDependentScalarField3d&&) = delete;
  TimeDependentScalarField3d& operator=(TimeDependentScalarField3d&&) = delete;

  /// Function to set the ODESolver. See enum MFEMODESolver for options.
  void SetODESolver(int solver_id);
  /// Set the operator, e.g. define the PDE to solve.
  void SetOperator(int operator_id);

  /// Execute one ODE timestep `dt`, e.g. compute `u(t+dt)` from `u(t)`.
  void Step(double dt);

  /// Update the grid function. The ODE procedure operates on the coefficient
  /// vector u_ and updates it. Before making calls to u_gf_, this routine
  /// must be called to update it.
  void UpdateGridFunction();

  /// Print information about the PDE / Continuum model
  void PrintInfo(std::ostream& out);

  /// Export the current continuum solution to the vtk format for ParaView.
  void ExportVTK();

  /// Forgets the previous table_of_elements_ and creates an up-to-date version.
  /// Use with caution as this call is expensive and you're likely to not have
  /// to use it.
  void UpdateElementToVertexTable();

  /// Returns true if position is contained in the element labeled with
  /// finite_element_id in the mesh_. You must also supply an
  /// mfem::IntegrationPoint in which the function stores the appropriate
  /// IntegrationPoint associated to the element to evaluate the GridFunction.
  bool ContainedInElement(const Double3& position, int finite_element_id,
                          mfem::IntegrationPoint& ip);

  /// Returns the id of the neighbor if position is contained in on of the
  /// neighbor elements of finite_element_id in the mesh_. If not, it returns
  /// INT_MAX.You must also supply an mfem::IntegrationPoint in which the
  /// function stores the appropriate IntegrationPoint associated to the element
  /// to evaluate the GridFunction.
  int ContainedInNeighbors(const Double3& position, int finite_element_id,
                           mfem::IntegrationPoint& ip);

  /// Returns a pair, where the integer is the finite element index in which the
  /// position was located and the double is the value of the GridFunction at
  /// the specified position. We recommend to store the finite element index
  /// since computing this index is expensive. If the index has been computed
  /// once, you can supply in via the finite_element_id variable to this
  /// function which makes it significantly faster.
  /// Warning: This is currently not very performant. If no position is
  /// provide, we have to search all elements.
  std::pair<int, double> GetSolutionAtPosition(
      const Double3& position,
      int finite_element_id = std::numeric_limits<int>::max());

  /// Get the value of the solution at the agents position. The function calls
  /// back to GetSolutionAtPosition and updates the Agent's member fe_id
  /// appropriately.
  double GetSolutionAtAgentPosition(Agent* agent);

  /// Set the PDE operator for the method of lines, e.g. define the equation.
  void SetOperator(MolOperator* oper);

  /// Get reference to fespace_, needed for custom operator
  mfem::FiniteElementSpace& GetFESpace() { return fespace_; }

  // Get the ODESolver associated to the MOL solver
  mfem::ODESolver* GetODESolver() { return ode_solver_; }

  // Get the MOLOperator associated to the MOL solver
  MolOperator* GetMolOperator() { return operator_; }

  // Get simulated time
  double GetSimTime() { return t_; }

  // Get a pointer to the GridFunction u_gf_
  mfem::GridFunction* GetGridFunction() { return &u_gf_; }

  // Get substance_id_
  uint64_t GetSubstanceId() { return substance_id_; }

  // Set substance_id_
  void SetSubstanceId(uint64_t id) { substance_id_ = id; }

  // Get substance_name_
  std::string GetSubstanceName() { return substance_name_; }

  // Set substance_name_
  void SetSubstanceName(std::string name) { substance_name_ = name; }
};

}  // namespace experimental
}  // namespace bdm

#endif  // MFEM_MOL_H_
#endif  // USE_MFEM
