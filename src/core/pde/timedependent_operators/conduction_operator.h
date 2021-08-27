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
#ifndef CONDUCTION_OPERATOR_H
#define CONDUCTION_OPERATOR_H

#include <fstream>
#include <iostream>
#include "core/pde/timedependent_operators/mol_operator.h"

namespace bdm {
namespace experimental {

/// This operator is generalized from MFEM EX16.
/// It corresponds to the equation:
/// \f[ \frac{du}{dt} = \nabla \cdot (\kappa + \alpha u) \nabla u \f]
/// After spatial discretization, the conduction model can be written as:
///
///   \f[ du/dt = M^{-1}(-Ku) \f]
///
/// where u is the vector representing the temperature, M is the mass matrix,
/// and K is the diffusion operator with diffusivity depending on u:
/// \f$ D = \kappa + \alpha u \f$.
///
/// The Class ConductionOperator represents the right-hand side of the above
/// ODE.
class ConductionOperator : public MolOperator {
 protected:
  double alpha_, kappa_;

 public:
  /// Constructor for full PDE
  /// \f$ \frac{du}{dt} = \nabla \cdot (\kappa + \alpha u) \nabla u \f$
  ConductionOperator(mfem::FiniteElementSpace &f, double alpha, double kappa);

  /// Update the BilinearForm K using the given true-dof vector `u`.
  void SetParameters(const mfem::Vector &u) override;
};

}  // namespace experimental
}  // namespace bdm

#endif  // CONDUCTION_OPERATOR_H
#endif  // USE_MFEM
