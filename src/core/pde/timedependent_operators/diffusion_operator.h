#ifndef DIFFUSION_OPERATOR_H
#define DIFFUSION_OPERATOR_H

#include <fstream>
#include <functional>
#include <iostream>
#include "core/pde/timedependent_operators/mol_operator.h"

namespace bdm {
namespace experimental {

/// This Operator descibes the following PDE system:
/// \f[ \frac{du}{dt} = \nabla (D \nabla u) + \Gamma u, \ \f]
/// where \f$ u = u(x,t) \f$, \f$ \Gamma = \Gamma (x,t)\f$, \f$ D = const \f$,
/// \f$ x \in R^n \f$. In the code, \f$ D \f$  is the variable
/// `diffusion_coefficient` and \f$\Gamma(x,t) = \Gamma(x)\f$ is called
/// `diffusion_func_`. Note that the implemenation currently does not support
/// the time-dependence in \f$ \Gamma \f$.
///
/// After spatial discretization, the diffusion model can be written as:
///
///   $f[ du/dt = M^{-1}(Ku) $f]
///
/// where \f$ u \f$ is the vector representing the concentration, \f$ M \f$ is
/// the mass matrix, and \f$ K \f$ is the diffusion operator with diffusivity
/// depending on `diffusion_coefficient` and a function `diffusion_func_`.
///
/// The Class DiffusionOperator represents the right-hand side of the above ODE.
class DiffusionOperator : public MolOperator {
 protected:
  double diff_coef_;
  bool scalar_coefficient_;
  std::function<double(const mfem::Vector &)> diffusion_func_;
  bool fuction_coefficient_;

 public:
  DiffusionOperator(
      mfem::FiniteElementSpace &f, double diffusion_coefficient,
      std::function<double(const mfem::Vector &)> diffusion_func_);
  DiffusionOperator(mfem::FiniteElementSpace &f, double diffusion_coefficient);
  DiffusionOperator(
      mfem::FiniteElementSpace &f,
      std::function<double(const mfem::Vector &)> diffusion_func_);

  /// Update the diffusion BilinearForm K using the given true-dof vector `u`.
  void SetParameters(const mfem::Vector &u);
};

}  // namespace experimental
}  // namespace bdm

#endif  // DIFFUSION_OPERATOR_H
