// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & University of Surrey for the benefit of the
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
#include "conduction_operator.h"

namespace bdm {
namespace experimental {

ConductionOperator::ConductionOperator(mfem::FiniteElementSpace &f,
                                       double alpha, double kappa)
    : MolOperator(f), alpha_(alpha), kappa_(kappa) {}

void ConductionOperator::SetParameters(const mfem::Vector &u) {
  mfem::GridFunction u_alpha_gf(&fespace_);
  u_alpha_gf.SetFromTrueDofs(u);
  for (int i = 0; i < u_alpha_gf.Size(); i++) {
    u_alpha_gf(i) = kappa_ + alpha_ * u_alpha_gf(i);
  }

  delete K_;
  K_ = new mfem::BilinearForm(&fespace_);

  mfem::GridFunctionCoefficient u_coeff(&u_alpha_gf);

  K_->AddDomainIntegrator(new mfem::DiffusionIntegrator(u_coeff));
  K_->Assemble();
  K_->FormSystemMatrix(ess_tdof_list_, Kmat_);
  delete T_;
  T_ = nullptr;  // re-compute T on the next ImplicitSolve
}

}  // namespace experimental
}  // namespace bdm

#endif  // USE_MFEM
