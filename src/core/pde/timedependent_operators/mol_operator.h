#ifdef USE_MFEM
#ifndef MOL_OPERATOR_H
#define MOL_OPERATOR_H

#include <fstream>
#include <iostream>
#include "mfem.hpp"

namespace bdm {
namespace experimental {

/// The Mol in MolOperator stands for Methods of Lines. Many reactio-diffusion
/// systems can be described with such an approach. This operator isolates the
/// the FE part of the Method of Lines and creates the matrices for the ODE
/// problem. It can supports explicit and implicit time integration if the user
/// decide to implement it or if his/her use case is close enough to the
/// follwoing.
///
/// The class is designed such that it handles MOLsystems that take an ODE shape
/// as:
/// \f[ M \cdot \frac{du(t)}{dt} = - K u(t) \f]
/// where \f$ u \f$ is vector of time dependent coefficients and \f$ M \f$ is
/// the mass matrix. The class can be modified and extended but for all cases
/// that deviate strongly from this form, some additional work is necessary.
/// Idealy, the user should only worry about the SetParameters() function to
/// update K. If K is time-independent, consider building K once in the
/// constructor and define SetParameters(){return;}.
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

  mutable mfem::Vector z_;  // auxiliary vector

  double last_dt_;

 public:
  MolOperator(mfem::FiniteElementSpace &f)
      : TimeDependentOperator(f.GetTrueVSize(), 0.0),
        fespace_(f),
        M_(nullptr),
        K_(nullptr),
        T_(nullptr),
        z_(height) {
    M_ = new mfem::BilinearForm(&fespace_);
    M_->AddDomainIntegrator(new mfem::MassIntegrator());
    M_->Assemble();
    M_->FormSystemMatrix(ess_tdof_list_, Mmat_);

    // For explicit solution
    const double rel_tol = 1e-8;
    M_solver_.iterative_mode = false;
    M_solver_.SetRelTol(rel_tol);
    M_solver_.SetAbsTol(0.0);
    M_solver_.SetMaxIter(30);
    M_solver_.SetPrintLevel(0);
    M_solver_.SetPreconditioner(M_prec_);
    M_solver_.SetOperator(Mmat_);

    // For implicit solution
    T_solver_.iterative_mode = false;
    T_solver_.SetRelTol(rel_tol);
    T_solver_.SetAbsTol(0.0);
    T_solver_.SetMaxIter(100);
    T_solver_.SetPrintLevel(0);
    T_solver_.SetPreconditioner(T_prec_);
  }

  /// This function will be called by explicit solvers. You will have to modify
  /// this function if you are not able to represent your system as in the class
  /// descrition.
  virtual void Mult(const mfem::Vector &u, mfem::Vector &du_dt) const override;

  /// Solve the Backward-Euler equation: k = f(u + dt*k, t), for the unknown k.
  /// This is the only requirement for high-order SDIRK implicit integration.
  /// You will have to modify this function if you are not able to represent
  /// your system as in the class descrition.
  virtual void ImplicitSolve(double dt, const mfem::Vector &u,
                             mfem::Vector &du_dt) override;

  /// Update the diffusion BilinearForm K using the given true-dof vector `u`.
  /// Here you must sepcify how to compute the matrix `K` for each timestep. If
  /// the matrix `K` does not change over time, add an additional check to your
  /// derived class such that it only computes K in the first step.
  virtual void SetParameters(const mfem::Vector &u);

  ~MolOperator() override;
};

}  // namespace experimental
}  // namespace bdm

#endif  // MOL_OPERATOR_H
#endif  // MFEM
