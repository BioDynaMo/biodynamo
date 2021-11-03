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
#include "diffusion_operator.h"
#include <stdexcept>

namespace bdm {
namespace experimental {

DiffusionOperator::DiffusionOperator(
    mfem::FiniteElementSpace &f, double diffusion_coefficient,
    std::function<double(const mfem::Vector &)> diffusion_func)
    : MolOperator(f),
      diff_coef_(diffusion_coefficient),
      scalar_coefficient_(true),
      diffusion_func_(std::move(diffusion_func)),
      fuction_coefficient_(true) {}

DiffusionOperator::DiffusionOperator(mfem::FiniteElementSpace &f,
                                     double diffusion_coefficient)
    : MolOperator(f),
      diff_coef_(diffusion_coefficient),
      scalar_coefficient_(true),
      fuction_coefficient_(false) {}

DiffusionOperator::DiffusionOperator(
    mfem::FiniteElementSpace &f,
    std::function<double(const mfem::Vector &)> diffusion_func)
    : MolOperator(f),
      scalar_coefficient_(false),
      diffusion_func_(std::move(diffusion_func)),
      fuction_coefficient_(true) {}

void DiffusionOperator::SetParameters(const mfem::Vector &u) {
  // Right now this is not really necessary since the function does not
  // changeover time, but once we sync it with the ABM, we want to update the
  // function every X timesteps.
  delete K_;
  K_ = new mfem::BilinearForm(&fespace_);

  if (scalar_coefficient_) {
    mfem::ConstantCoefficient diff_coeff(diff_coef_);
    K_->AddDomainIntegrator(new mfem::DiffusionIntegrator(diff_coeff));
  }
  if (fuction_coefficient_) {
    mfem::FunctionCoefficient func_coeff(diffusion_func_);
    K_->AddDomainIntegrator(new mfem::MassIntegrator(func_coeff));
  }
  K_->Assemble();
  K_->FormSystemMatrix(ess_tdof_list_, Kmat_);
  delete T_;
  T_ = nullptr;  // re-compute T on the next ImplicitSolve
}

}  // namespace experimental
}  // namespace bdm

#endif  // USE_MFEM
