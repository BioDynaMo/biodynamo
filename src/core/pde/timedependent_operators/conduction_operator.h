#ifndef CONDUCTION_OPERATOR_H
#define CONDUCTION_OPERATOR_H

#include <fstream>
#include <iostream>
#include "core/pde/timedependent_operators/mol_operator.h"

namespace bdm {
namespace experimental {

/// This operator is taken from MFEM EX16.
/// It corresponds to the equation:
/// \frac{du}{dt} = \nabla \cdot (\kappa + \alpha u) \nabla u
/// After spatial discretization, the conduction model can be written as:
///
///    du/dt = M^{-1}(-Ku)
///
/// where u is the vector representing the temperature, M is the mass matrix,
/// and K is the diffusion operator with diffusivity depending on u:
/// (\kappa + \alpha u).
///
/// The Class ConductionOperator represents the right-hand side of the above
/// ODE.
class ConductionOperator : public MolOperator {
 protected:
  double alpha_, kappa_;

 public:
  ConductionOperator(mfem::FiniteElementSpace &f, double alpha, double kappa);

  void SetParameters(const mfem::Vector &u);
};

}  // namespace experimental
}  // namespace bdm

#endif  // CONDUCTION_OPERATOR_H
