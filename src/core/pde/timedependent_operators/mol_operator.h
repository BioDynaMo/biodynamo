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
#ifndef MOL_OPERATOR_H
#define MOL_OPERATOR_H

#include <fstream>
#include <iostream>
#include "core/agent/cell.h"
#include "mfem.hpp"

namespace bdm {
namespace experimental {

class AccumulateDoubleFunctor : public Functor<void, Agent *, double> {
 private:
  Cell *query_;
  double accumulated_value_;
  double norm_;
  std::function<double(double, double)> density_from_distance_;

 public:
  AccumulateDoubleFunctor(
      double norm, std::function<double(double, double)> density_from_distance =
                       [](double distance, double query_radius) {
                         return (distance < query_radius) ? 1 : 0;
                       })
      : accumulated_value_(0.0),
        norm_(norm),
        density_from_distance_(std::move(density_from_distance)) {}

  void operator()(Agent *neighbor, double squared_distance) {
    auto *neighbor_cell = bdm_static_cast<Cell *>(neighbor);
    if (neighbor_cell == query_) {
      return;
    }
    auto squared_cell_radius = pow(neighbor_cell->GetDiameter() / 2, 2);
    auto agent_contribution_to_density =
        density_from_distance_(squared_distance, squared_cell_radius);
#pragma omp atomic
    accumulated_value_ += agent_contribution_to_density;
  }

  double GetAccumulatedValue() { return accumulated_value_ * norm_; }

  void Reset() {
    accumulated_value_ = 0.0;
    query_ = nullptr;
  }

  void SetNorm(double norm) { norm_ = norm; }
  double GetNorm() { return norm_; }

  void SetQueryAgent(Cell *query) {
    Reset();
    query_ = query;
  }

  void SetDensityFunction(std::function<double(double, double)> df) {
    density_from_distance_ = std::move(df);
  }
};

/// The Mol in MolOperator stands for Methods of Lines. Many reactio-diffusion
/// systems can be described with such an approach. This operator isolates the
/// the FE part of the Method of Lines and creates the matrices for the ODE
/// problem. It can supports explicit and implicit time integration if the user
/// decide to implement it or if his/her use case is close enough to the
/// follwoing.
///
/// The class is designed such that it handles MOL systems that take an ODE
/// shape as:
/// \f[ M \cdot \frac{du(t)}{dt} = - K(t) u(t) \ , \f]
/// where \f$ u \f$ is vector of the time dependent FE-expansion coefficients
/// and \f$ M \f$ is the mass matrix. The class can be modified and extended but
/// for all cases that deviate strongly from this form, some additional work is
/// necessary. Ideally, the user should only worry about the SetParameters()
/// function to update K. If K is time-independent, consider building K once in
/// the constructor and define SetParameters(){return;}.
class MolOperator : public mfem::TimeDependentOperator {
 protected:
  // this list remains empty for pure Neumann b.c.
  mfem::Array<int> ess_tdof_list_;
  mfem::FiniteElementSpace &fespace_;
  mfem::BilinearForm *M_;
  mfem::BilinearForm *K_;

  mfem::SparseMatrix Mmat_, Kmat_;
  mfem::SparseMatrix *T_;  // T = M + dt K, needed for implicit solve

  mfem::CGSolver M_solver_;  // Krylov solver for inverting the mass matrix M
  mfem::DSmoother M_prec_;   // Preconditioner for the mass matrix M

  mfem::CGSolver T_solver_;  // Implicit solver for T = M + dt K
  mfem::DSmoother T_prec_;   // Preconditioner for the implicit solver

  mutable mfem::Vector z_;  // auxiliary vector for internal calculations

  double last_dt_;

  AccumulateDoubleFunctor compute_agent_pdf_functor_;

 public:
  MolOperator(mfem::FiniteElementSpace &f)
      : TimeDependentOperator(f.GetTrueVSize(), 0.0),
        fespace_(f),
        M_(nullptr),
        K_(nullptr),
        T_(nullptr),
        z_(height),
        compute_agent_pdf_functor_(1.0) {
    // Generate Mass Matrix
    M_ = new mfem::BilinearForm(&fespace_);
    M_->AddDomainIntegrator(new mfem::MassIntegrator());
    M_->Assemble();
    M_->FormSystemMatrix(ess_tdof_list_, Mmat_);

    // Define solver for inverting the mass matrix for explicit solution
    const double rel_tol = 1e-8;
    M_solver_.iterative_mode = false;
    M_solver_.SetRelTol(rel_tol);
    M_solver_.SetAbsTol(0.0);
    M_solver_.SetMaxIter(30);
    M_solver_.SetPrintLevel(0);
    M_solver_.SetPreconditioner(M_prec_);
    M_solver_.SetOperator(Mmat_);

    // Define another solver for matrix inversion for implicit solution
    T_solver_.iterative_mode = false;
    T_solver_.SetRelTol(rel_tol);
    T_solver_.SetAbsTol(0.0);
    T_solver_.SetMaxIter(100);
    T_solver_.SetPrintLevel(0);
    T_solver_.SetPreconditioner(T_prec_);
  }
  /// Destructor deltes instances created with `new` in the member functions and
  /// the constructor to avoid memory leaks.
  virtual ~MolOperator() override;

  /// This function is called by explicit solvers. You have to modify
  /// is if you are not able to represent your system as in the class
  /// descrition. Consult the documentation of mfem::ODESolver to see how the
  /// member function mfem::TimeDependentOperator::Mult() is used by an
  /// mfem::ODESolver.
  virtual void Mult(const mfem::Vector &u, mfem::Vector &du_dt) const override;

  /// Solve the Backward-Euler equation: k = f(u + dt*k, t), for the unknown k.
  /// This is the only requirement for high-order SDIRK implicit integration.
  /// You will have to modify this function if you are not able to represent
  /// your system as in the class descrition. Consult the documentation of
  /// mfem::ODESolver to see how the member function
  /// mfem::TimeDependentOperator::ImplicitSolve() is used by an
  /// mfem::ODESolver.
  virtual void ImplicitSolve(double dt, const mfem::Vector &u,
                             mfem::Vector &du_dt) override;

  /// Update the diffusion BilinearForm K(t) using the given true-dof vector
  /// `u`. Here you must sepcify how to compute the matrix K_ for each
  /// timestep. If the matrix K_ does not change over time, add an additional
  /// check to your derived class such that it only computes K in the first
  /// step.
  virtual void SetParameters(const mfem::Vector &u);

  /// Evaluates the AgentPDF at postion x. The AgentPDF is here defined as
  /// \[f p(x) = \left( /sum_i V_i \right)^{-1} \f]
  /// if x is inside of an agent and zero else. \f$ V_i \f$ is the volume of
  /// agent-i and, thus, the volume integral over the entire domain
  /// \[f \int_\Omega p(y) dy = 1 \f]
  /// if we assume agents that do not overlap.
  double EvaluateAgentPDF(const mfem::Vector &x);

  /// Get pointer to the internal AccumulateDoubleFunctor responsible for
  /// computing the AgentPDF. This can be used to create your custom PDF via
  /// auto* f = MolOperator::GetAgentPDFFunctor();
  /// f->SetDensityFunction(MyFunction);
  AccumulateDoubleFunctor *GetAgentPDFFunctor() {
    return &compute_agent_pdf_functor_;
  }

  /// Update the normalization.
  void UpdatePDFNorm();
};

}  // namespace experimental
}  // namespace bdm

#endif  // MOL_OPERATOR_H
#endif  // MFEM
