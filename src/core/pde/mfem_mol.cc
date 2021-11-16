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
#include "mfem_mol.h"
#include "core/util/log.h"
#include "mfem.hpp"

namespace bdm {
namespace experimental {

mfem::Vector ConvertToMFEMVector(const Double3& position) {
  mfem::Vector vec({position[0], position[1], position[2]});
  return vec;
}

TimeDependentScalarField3d::TimeDependentScalarField3d(
    mfem::Mesh* mesh, int order, int dimension, MFEMODESolver ode_solver_id,
    PDEOperator pde_oper_id,
    std::function<double(const mfem::Vector&)> InitialGridValues,
    std::vector<double> numeric_operator_parameters,
    std::vector<std::function<double(const mfem::Vector&)>> operator_functions)
    : fe_coll_(order, dimension),
      mesh_(mesh),
      fespace_(mesh_, &fe_coll_),
      u_gf_(&fespace_),
      ode_solver_(nullptr),
      element_finder_(mesh),
      operator_(nullptr),
      InitialGridValues_(std::move(InitialGridValues)),
      numeric_operator_parameters_(std::move(numeric_operator_parameters)),
      operator_functions_(std::move(operator_functions)),
      t_(0.0),
      dt_max_(std::numeric_limits<double>::max()),
      ode_steps_(0),
      gf_tdof_in_sync_(true) {
  Initialize();
  SetOperator(pde_oper_id);
  SetODESolver(ode_solver_id);
}

TimeDependentScalarField3d::~TimeDependentScalarField3d() {
  // ODE operator and ODE solver are generated withing this class with new so
  // we call the delete operator for them. Note that the mesh is typically
  // generated in the ModelInitializer or in the user defined code and the
  // delete operator is called for all registed meshes in Simulation.
  delete ode_solver_;
  delete operator_;
}

void TimeDependentScalarField3d::Initialize() {
  // The vector u_ basically represents the coefficients in the finite element
  // expansion. These coefficients form the ODE system in the method of lines.
  mfem::FunctionCoefficient u_0(InitialGridValues_);
  u_gf_.ProjectCoefficient(u_0);
  u_gf_.GetTrueDofs(u_);
  u_gf_.SetFromTrueDofs(u_);
}

void TimeDependentScalarField3d::SetBoundaryConditions() {
  // ToDo(tobias): Implement Interface for Neuman and Dirichlet Boundary
  // conditions.
  return;
}

void TimeDependentScalarField3d::SetODESolver(int solver_id) {
  if (ode_solver_ != nullptr) {
    delete ode_solver_;
  }
  switch (solver_id) {
    // Implicit L-stable methods
    case MFEMODESolver::kBackwardEulerSolver:
      ode_solver_ = new mfem::BackwardEulerSolver;
      break;
    case MFEMODESolver::kSDIRK23Solver2:
      ode_solver_ = new mfem::SDIRK23Solver(2);
      break;
    case MFEMODESolver::kSDIRK33Solver:
      ode_solver_ = new mfem::SDIRK33Solver;
      break;
    // Explicit methods
    case MFEMODESolver::kForwardEulerSolver:
      ode_solver_ = new mfem::ForwardEulerSolver;
      break;
    case MFEMODESolver::kRK2Solver:
      ode_solver_ = new mfem::RK2Solver(0.5);
      break;  // midpoint method
    case MFEMODESolver::kRK3SSPSolver:
      ode_solver_ = new mfem::RK3SSPSolver;
      break;
    case MFEMODESolver::kRK4Solver:
      ode_solver_ = new mfem::RK4Solver;
      break;
    case MFEMODESolver::kGeneralizedAlphaSolver:
      ode_solver_ = new mfem::GeneralizedAlphaSolver(0.5);
      break;
    // Implicit A-stable methods (not L-stable)
    case MFEMODESolver::kImplicitMidpointSolver:
      ode_solver_ = new mfem::ImplicitMidpointSolver;
      break;
    case MFEMODESolver::kSDIRK23Solver1:
      ode_solver_ = new mfem::SDIRK23Solver;
      break;
    case MFEMODESolver::kSDIRK34Solver:
      ode_solver_ = new mfem::SDIRK34Solver;
      break;
    // In case the user provides a faulty solver_id, we terminate the program
    // in the following way.
    default:
      Log::Fatal("TimeDependentScalarField3d::SetODESolver",
                 "Unknown ODE solver type: ", solver_id);
  }
  // Initialize the ODE solver with the operator.
  if (operator_ != nullptr && ode_solver_ != nullptr) {
    ode_solver_->Init(*operator_);
  } else {
    Log::Fatal("TimeDependentScalarField3d::SetODESolver",
               "Cannot initialize ode_solver wiht nullptr as operator.");
  }
}

void TimeDependentScalarField3d::SetOperator(int operator_id) {
  if (operator_ != nullptr) {
    delete operator_;
  }
  switch (operator_id) {
    case PDEOperator::kDiffusion:
      if (numeric_operator_parameters_.size() < 1) {
        Log::Fatal(
            "TimeDependentScalarField3d::SetOperator",
            "Wrong number of numerical parameters for DiffusionOperator.",
            "\nExpected: 1, Acutal: ", numeric_operator_parameters_.size());
      }
      operator_ =
          new DiffusionOperator(fespace_, numeric_operator_parameters_[0]);
      break;
    case PDEOperator::kDiffusionPerformance:
      if (numeric_operator_parameters_.size() < 1) {
        Log::Fatal(
            "TimeDependentScalarField3d::SetOperator",
            "Wrong number of numerical parameters for kDiffusionPerformance.",
            "\nExpected: 1, Acutal: ", numeric_operator_parameters_.size());
      }
      operator_ = new DiffusionOperatorPerformance(
          fespace_, numeric_operator_parameters_[0]);
      break;
    case PDEOperator::kDiffusionWithFunction:
      if (operator_functions_.size() < 1 ||
          numeric_operator_parameters_.size() < 1) {
        Log::Fatal(
            "TimeDependentScalarField3d::SetOperator",
            "Wrong number of functions for DiffusionOperator:",
            "\nExpected: 1, Acutal: ", operator_functions_.size(),
            "\nor wrong number of numerical parameters:",
            "\nExpected: 1, Acutal: ", numeric_operator_parameters_.size());
      }
      operator_ = new DiffusionOperator(
          fespace_, numeric_operator_parameters_[0], operator_functions_[0]);
      break;
    case PDEOperator::kDiffusionWithFunctionPerformance:
      if (operator_functions_.size() < 1 ||
          numeric_operator_parameters_.size() < 1) {
        Log::Fatal(
            "TimeDependentScalarField3d::SetOperator",
            "Wrong number of functions for DiffusionOperator:",
            "\nExpected: 1, Acutal: ", operator_functions_.size(),
            "\nor wrong number of numerical parameters:",
            "\nExpected: 1, Acutal: ", numeric_operator_parameters_.size());
      }
      operator_ = new DiffusionOperatorPerformance(
          fespace_, numeric_operator_parameters_[0], operator_functions_[0]);
      break;
    case PDEOperator::kConduction:
      if (numeric_operator_parameters_.size() < 2) {
        Log::Fatal(
            "TimeDependentScalarField3d::SetOperator",
            "Wrong number of numerical parameters for ConductionOperator.",
            "\nExpected: 2, Acutal: ", numeric_operator_parameters_.size());
      }
      operator_ =
          new ConductionOperator(fespace_, numeric_operator_parameters_[0],
                                 numeric_operator_parameters_[1]);
      break;
    default:
      Log::Fatal("TimeDependentScalarField3d::SetOperator",
                 "Unknown ODE solver type: ", operator_id);
  }
}

// Possibly this method should be based on mfem::ODESolver::Run().
void TimeDependentScalarField3d::Step(double dt) {
  operator_->SetParameters(u_);
  const double t_target = t_ + dt;
  double time_step = std::min(dt, dt_max_);
  // Run() uses automatic time stepping.
  ode_solver_->Run(u_, t_, time_step, t_target);
  if (t_target != t_) {
    Log::Warning("TimeDependentScalarField3d::Step",
                 "Call to MFEM::ODESolver behaved not as expected.\n",
                 "\nTarget time: ", t_, " / ", t_target, "\n(is / expected)");
  }
  // The step function updates the tdof vector u_, thus, u_gf_ does not contain
  // the latest information of the Simulation any longer. Once we start reading
  // off values from the u_gf_ again, we need to update it. Here, we set a flag
  // such that we know that we have to update it.
  gf_tdof_in_sync_ = false;
  ode_steps_++;
}

void TimeDependentScalarField3d::VerifyBDMCompatibility() {
  auto* param = Simulation::GetActive()->GetParam();
  // Check if the parameter space is bounded.
  if (param->bound_space != Param::BoundSpaceMode::kClosed) {
    Log::Fatal("TimeDependentScalarField3d::VerifyBDMCompatibility",
               "Please make sure to use Param::BoundSpaceMode::kClosed.");
  }
  // Check if the MFEM::Grid contains all corners of the BDM simulation cube.
  double min = param->min_bound;
  double max = param->max_bound;
  std::vector<Double3> bdm_cube_corners{
      {min, min, min}, {min, min, max}, {min, max, min}, {max, min, min},
      {min, max, max}, {max, min, max}, {max, max, min}, {max, max, max}};
  for (auto& pos : bdm_cube_corners) {
    // This function throws a fatal itself if the point `pos` is not found
    // inside the MFEM mesh.
    element_finder_.FindPointWithOctree(pos);
  }
}

void TimeDependentScalarField3d::UpdateGridFunction() {
  if (!gf_tdof_in_sync_) {
    u_gf_.SetFromTrueDofs(u_);
    gf_tdof_in_sync_ = true;
  }
}

void TimeDependentScalarField3d::PrintInfo(std::ostream& out) {
  out << std::string(80, '_') << "\n";
  out << "\nSubstance ID\t\t\t: " << substance_id_ << "\n";
  out << "Substance name\t\t\t: " << substance_name_ << "\n";
  std::string object_name;
  object_name = typeid(*ode_solver_).name();
  out << "ODE Solver\t\t\t: " << object_name << "\n";
  object_name = typeid(*operator_).name();
  out << "PDE Operator\t\t\t: " << object_name << "\n";
  out << "Finite Element Collection\t: " << fe_coll_.Name() << "\n";
  out << "Polynomial degree (FE)\t\t: " << fe_coll_.GetOrder() << "\n";
  out << "ODE dimension\t\t\t: " << u_.Size() << "\n";
  out << "ODE steps executed\t\t: " << ode_steps_ << "\n";
  out << "ODE simulated time\t\t: " << t_ << "\n";
  mesh_->PrintInfo(out);
  out << std::string(80, '_') << "\n";
}

void ExportVTK() {
  // ToDo(tobias): Export mesh + values to paraview and integrate into BDM
  // Paraview adaptor.
  return;
}

void TimeDependentScalarField3d::SetOperator(MolOperator* oper) {
  if (operator_ != nullptr && operator_ != oper) {
    delete operator_;
  }
  operator_ = oper;
  // Initialize the ODE solver with the operator.
  if (operator_ != nullptr && ode_solver_ != nullptr) {
    ode_solver_->Init(*operator_);
  } else {
    Log::Fatal("TimeDependentScalarField3d::SetOperator",
               "Cannot initialize ode_solver wiht nullptr as operator.");
  }
}

double TimeDependentScalarField3d::GetSolutionInElementAndIntegrationPoint(
    int element_id, const mfem::IntegrationPoint& integration_point) {
  return u_gf_.GetValue(element_id, integration_point);
}

std::pair<int, double> TimeDependentScalarField3d::GetSolutionAtPosition(
    const Double3& position, int finite_element_id) {
  mfem::IntegrationPoint integration_point;
  // To read off the values, the grid function must be up to date with the tdof
  // vector used to solve the ODE.
  UpdateGridFunction();

  // We initiate a search with the an octree implementation to identify the
  // containing element and the closest integration point to the position. The
  // search is biased in the sense that we first check the provided
  // finite_element_id and its neighbors. Only if these checks fail, we start a
  // search based on the octree.
  auto search =
      element_finder_.FindPointWithOctree(position, finite_element_id);
  finite_element_id = search.first;
  integration_point = search.second;
  double grid_value = GetSolutionInElementAndIntegrationPoint(
      finite_element_id, integration_point);
  return std::make_pair(finite_element_id, grid_value);
}

double TimeDependentScalarField3d::GetSolutionAtAgentPosition(Agent* agent) {
  auto result =
      GetSolutionAtPosition(agent->GetPosition(), agent->GetFiniteElementID());
  agent->SetFiniteElementID(result.first);
  return result.second;
}

}  // namespace experimental
}  // namespace bdm

#endif  // USE_MFEM
