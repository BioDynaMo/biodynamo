#include "mfem_mol.h"
#include "core/util/log.h"
#include "mfem.hpp"

namespace bdm {
namespace experimental {

MethodOfLineSolver::MethodOfLineSolver(
    mfem::Mesh* mesh, int order, int dimension, int ode_solver_id,
    int pde_oper_id,
    std::function<double(const mfem::Vector&)> InitialGridValues,
    std::vector<double> numeric_operator_parameters,
    std::vector<std::function<double(const mfem::Vector&)>> operator_functions)
    : fe_coll_(order, dimension),
      mesh_(mesh),
      fespace_(mesh_, &fe_coll_),
      u_gf_(&fespace_),
      ode_solver_(nullptr),
      operator_(nullptr),
      InitialGridValues_(std::move(InitialGridValues)),
      numeric_operator_parameters_(std::move(numeric_operator_parameters)),
      operator_functions_(std::move(operator_functions)),
      t_(0.0) {
  Initialize();
  SetOperator(pde_oper_id);
  SetODESolver(ode_solver_id);
}

MethodOfLineSolver::~MethodOfLineSolver() {
  delete mesh_;
  delete ode_solver_;
  delete operator_;
}

void MethodOfLineSolver::Initialize() {
  mfem::FunctionCoefficient u_0(InitialGridValues_);
  u_gf_.ProjectCoefficient(u_0);
  u_gf_.GetTrueDofs(u_);
  u_gf_.SetFromTrueDofs(u_);
}

void MethodOfLineSolver::SetBoundaryConditions() {
  // ToDo(tobias): Implement Interface for Neuman and Dirichlet Boundary
  // conditions.
  return;
}

void MethodOfLineSolver::SetODESolver(int solver_id) {
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
      Log::Fatal("MethodOfLineSolver::SetODESolver",
                 "Unknown ODE solver type: ", solver_id);
  }
  // Initialize the ODE solver with the operator.
  if (operator_ != nullptr && ode_solver_ != nullptr) {
    ode_solver_->Init(*operator_);
  } else {
    Log::Fatal("MethodOfLineSolver::SetODESolver",
               "Cannot initialize ode_solver wiht nullptr as operator.");
  }
}

void MethodOfLineSolver::SetOperator(int operator_id) {
  switch (operator_id) {
    case PDEOperator::kDiffusion:
      if (numeric_operator_parameters_.size() != 1) {
        Log::Fatal(
            "MethodOfLineSolver::SetOperator",
            "Wrong number of numerical parameters for DiffusionOperator.",
            "\nExpected: 1, Acutal: ", numeric_operator_parameters_.size());
      }
      operator_ =
          new DiffusionOperator(fespace_, numeric_operator_parameters_[0]);
      break;
    case PDEOperator::kDiffusionWithFunction:
      if (operator_functions_.size() != 1) {
        Log::Fatal("MethodOfLineSolver::SetOperator",
                   "Wrong number of functions for DiffusionOperator.",
                   "\nExpected: 1, Acutal: ", operator_functions_.size());
      }
      operator_ = new DiffusionOperator(fespace_, operator_functions_[0]);
      break;
    case PDEOperator::kConduction:
      if (numeric_operator_parameters_.size() != 2) {
        Log::Fatal(
            "MethodOfLineSolver::SetOperator",
            "Wrong number of numerical parameters for ConductionOperator.",
            "\nExpected: 2, Acutal: ", numeric_operator_parameters_.size());
      }
      operator_ =
          new ConductionOperator(fespace_, numeric_operator_parameters_[0],
                                 numeric_operator_parameters_[1]);
      break;
    default:
      Log::Fatal("MethodOfLineSolver::SetOperator",
                 "Unknown ODE solver type: ", operator_id);
  }
}

void MethodOfLineSolver::Step(double dt) {
  operator_->SetParameters(u_);
  const double dt_ref = dt;
  const double t_target = t_ + dt;
  ode_solver_->Step(u_, t_, dt);
  if (dt_ref != dt || t_target != t_) {
    Log::Warning("MethodOfLineSolver::Step",
                 "Call to MFEM::ODESolver behaved not as expected.\n",
                 "Time step: ", dt, " / ", dt_ref, "\nTarget time: ", t_, " / ",
                 t_target, "\n(is / expected)");
  }
}

void MethodOfLineSolver::Visualize() { return; }

void MethodOfLineSolver::UpdateGridFunction() { u_gf_.SetFromTrueDofs(u_); }

void MethodOfLineSolver::SetOperator(MolOperator* oper) {
  if (operator_ != nullptr && operator_ != oper) {
    delete operator_;
  }
  operator_ = oper;
  // Initialize the ODE solver with the operator.
  if (operator_ != nullptr && ode_solver_ != nullptr) {
    ode_solver_->Init(*operator_);
  } else {
    Log::Fatal("MethodOfLineSolver::SetOperator",
               "Cannot initialize ode_solver wiht nullptr as operator.");
  }
}

double MethodOfLineSolver::GetSolutionAtPosition() { return 0.0; }

}  // namespace experimental
}  // namespace bdm
