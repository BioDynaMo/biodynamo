#include "mol_operator.h"

namespace bdm {
namespace experimental {

void MolOperator::Mult(const mfem::Vector &u, mfem::Vector &du_dt) const {
  // This function is called by explicit ODE solver and therefore has to
  // compute:
  //    du_dt = M^{-1}*-K(u)
  // Input: u, Output du_dt
  Kmat_.Mult(u, z_);          // z = K*u
  z_.Neg();                   // z = -z
  M_solver_.Mult(z_, du_dt);  // Finds du_dt: M du_dt = z -> du_dt=M^{-1} z
}

void MolOperator::ImplicitSolve(const double dt, const mfem::Vector &u,
                                mfem::Vector &du_dt) {
  // Solve the equation:
  //    du_dt = M^{-1}*[-K(u + dt*du_dt)]
  // for du_dt
  if (T_ == nullptr) {
    T_ = Add(1.0, Mmat_, dt, Kmat_);
    last_dt_ = dt;
    T_solver_.SetOperator(*T_);
  }
  MFEM_VERIFY(dt == last_dt_, "");  // SDIRK methods use the same dt
  Kmat_.Mult(u, z_);
  z_.Neg();
  T_solver_.Mult(z_, du_dt);
}

MolOperator::~MolOperator() {
  delete T_;
  delete M_;
  delete K_;
}

}  // namespace experimental
}  // namespace bdm
