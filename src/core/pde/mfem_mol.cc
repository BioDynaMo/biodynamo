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

TimeDependentScalarField3d::TimeDependentScalarField3d(
    mfem::Mesh* mesh, int order, int dimension, MFEMODESolver ode_solver_id,
    PDEOperator pde_oper_id,
    std::function<double(const mfem::Vector&)> InitialGridValues,
    std::vector<double> numeric_operator_parameters,
    std::vector<std::function<double(const mfem::Vector&)>> operator_functions)
    : fe_coll_(order, dimension),
      mesh_(mesh),
      table_of_elements_(nullptr),
      fespace_(mesh_, &fe_coll_),
      u_gf_(&fespace_),
      ode_solver_(nullptr),
      operator_(nullptr),
      InitialGridValues_(std::move(InitialGridValues)),
      numeric_operator_parameters_(std::move(numeric_operator_parameters)),
      operator_functions_(std::move(operator_functions)),
      t_(0.0),
      dt_max_(std::numeric_limits<double>::max()),
      ode_steps_(0) {
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
  delete table_of_elements_;
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
    case PDEOperator::kDiffusionWithFunction:
      if (operator_functions_.size() < 1) {
        Log::Fatal("TimeDependentScalarField3d::SetOperator",
                   "Wrong number of functions for DiffusionOperator.",
                   "\nExpected: 1, Acutal: ", operator_functions_.size());
      }
      operator_ = new DiffusionOperator(fespace_, operator_functions_[0]);
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
  u_gf_.SetFromTrueDofs(u_);
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
    FindPointInMesh(pos);
  }
}

void TimeDependentScalarField3d::UpdateGridFunction() {
  u_gf_.SetFromTrueDofs(u_);
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

void TimeDependentScalarField3d::UpdateElementToVertexTable() {
  if (table_of_elements_ != nullptr) {
    delete table_of_elements_;
    table_of_elements_ = nullptr;
  }
  table_of_elements_ = mesh_->GetVertexToElementTable();
};

mfem::Vector ConvertToMFEMVector(const Double3& position) {
  mfem::Vector vec({position[0], position[1], position[2]});
  return vec;
}

bool TimeDependentScalarField3d::ContainedInElement(
    const Double3& position, int finite_element_id,
    mfem::IntegrationPoint& ip) {
  mfem::Vector vec = ConvertToMFEMVector(position);
  mfem::InverseElementTransformation inv_tr;
  auto trans = mesh_->GetElementTransformation(finite_element_id);
  inv_tr.SetTransformation(*trans);
  return (inv_tr.Transform(vec, ip) ==
          mfem::InverseElementTransformation::Inside);
}

int TimeDependentScalarField3d::ContainedInNeighbors(
    const Double3& position, int finite_element_id,
    mfem::IntegrationPoint& ip) {
  // If the search in the neighboring elements is not successful, we return
  // INT_MAX.
  int containing_neighbor_element = std::numeric_limits<int>::max();
  // This large table stores a mapping between Elements and Vertices. We only
  // want to construct it once and store it.
  if (table_of_elements_ == nullptr) {
    table_of_elements_ = mesh_->GetVertexToElementTable();
  }

  // Integer array in which we store the vertices of the element
  // finite_element_id. We iterate over all vertices.
  mfem::Array<int> vertices;
  mesh_->GetElementVertices(finite_element_id, vertices);
  for (int v = 0; v < vertices.Size(); v++) {
    // For each vertex, we get all elements that touch it / are connected
    // through it.
    int vertex = vertices[v];
    int number_of_elements = table_of_elements_->RowSize(vertex);
    const int* element_ids = table_of_elements_->GetRow(vertex);
    for (int e = 0; e < number_of_elements; e++) {
      // We don't want to check the original element as we want to scan the
      // neighbors.
      if (element_ids[e] == finite_element_id) {
        continue;
      }
      // For each element, we test if if it contains the position.
      bool is_inside = ContainedInElement(position, element_ids[e], ip);
      if (is_inside) {
        containing_neighbor_element = element_ids[e];
        break;
      }
    }
  }
  // Warning for non-conforming meshes
  if (mesh_->ncmesh &&
      containing_neighbor_element == std::numeric_limits<int>::max()) {
    Log::Warning(
        "TimeDependentScalarField3d::ContainedInNeighbors",
        "You seem to use a non-conforming mesh. Iterating over neighbors in ",
        "non-conforming meshes is currently not supported since it requires "
        "access",
        " to private / protected members of mfem::Mesh.");
  }
  return containing_neighbor_element;
}

std::pair<int, mfem::IntegrationPoint>
TimeDependentScalarField3d::FindPointInMesh(const Double3& position) {
  // This is not the most efficient way to transfer the information but
  // currently necessary.
  mfem::DenseMatrix mfem_position(mesh_->Dimension(), 1);
  for (int i = 0; i < mesh_->Dimension(); i++) {
    mfem_position(i, 0) = position[i];
  }
  mfem::Array<int> element_id;
  mfem::Array<mfem::IntegrationPoint> integration_points;
  // This function is particularly slow since it searches the space rather
  // inefficiently. It would be good to have something more efficient.
  auto found = mesh_->FindPoints(mfem_position, element_id, integration_points);
  if (found == 0) {
    Log::Fatal("FindPoints", "Point could not be located in Mesh. Point: (",
               position,
               ")\nIf you see this error message, this can be for either of "
               "some reasons:\n",
               "1) You manually called this function for a point that is not "
               "in the FE mesh\n"
               "2) Your agent simulation is not fully contained in the FE mesh "
               "- see VerifyBDMCompatibility()\n",
               "3) An Agent tried to request a value from the continuum in the",
               " syncronization phase but was not located inside the FE Mesh\n",
               "4) MFEM's internal methods failed to find the point for any ",
               "other reason.\n(This list may not be exhaustive.)");
  }
  return std::make_pair(element_id[0], integration_points[0]);
}

double TimeDependentScalarField3d::GetSolutionInElementAndIntegrationPoint(
    int element_id, const mfem::IntegrationPoint& integration_point) {
  return u_gf_.GetValue(element_id, integration_point);
}

std::pair<int, double> TimeDependentScalarField3d::GetSolutionAtPosition(
    const Double3& position, int finite_element_id) {
  mfem::IntegrationPoint integration_point;
  // 1. By default, each agent has its element set to INT_MAX. If the agent had
  // been treated before, the agent carries his previous element id. We first
  // check if the agent is still in the element.
  if (finite_element_id != std::numeric_limits<int>::max()) {
    bool in_element =
        ContainedInElement(position, finite_element_id, integration_point);
    // 2. In case the agent is no longer in the element, we check if
    // the agent ends up in one of the neighbouring elements
    if (!in_element) {
      finite_element_id =
          ContainedInNeighbors(position, finite_element_id, integration_point);
    }
  }
  // 3. If the position is neither in the expected nor in the neighboring
  // elements, we trigger a linear search over all elements.
  if (finite_element_id == std::numeric_limits<int>::max()) {
    auto search = FindPointInMesh(position);
    finite_element_id = search.first;
    integration_point = search.second;
  }
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
