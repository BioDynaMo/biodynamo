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

DiffusionOperatorPerformance::DiffusionOperatorPerformance(
    mfem::FiniteElementSpace &f, double diffusion_coefficient)
    : MolOperator(f),
      diffusion_coefficient_(diffusion_coefficient),
      assembled_(false),
      diffusion_function_(nullptr) {}

DiffusionOperatorPerformance::DiffusionOperatorPerformance(
    mfem::FiniteElementSpace &f, double diffusion_coefficient,
    std::function<double(const mfem::Vector &)> diffusion_func)
    : DiffusionOperatorPerformance(f, diffusion_coefficient) {
  mfem::FunctionCoefficient *mfem_diffusion_func =
      new mfem::FunctionCoefficient(diffusion_func);
  diffusion_function_ = mfem_diffusion_func;
}

DiffusionOperatorPerformance::~DiffusionOperatorPerformance() {
  if (diffusion_function_ != nullptr) {
    delete diffusion_function_;
  }
};

void DiffusionOperatorPerformance::SetParameters(const mfem::Vector &u) {
  // We only construct the Bilinear Form K (and also T_) once
  if (!assembled_) {
    // Define BilinearForm K_
    delete K_;
    K_ = new mfem::BilinearForm(&fespace_);
    K_->SetAssemblyLevel(mfem::AssemblyLevel::LEGACY);
    mfem::ConstantCoefficient diff_coeff(diffusion_coefficient_);
    K_->AddDomainIntegrator(new mfem::DiffusionIntegrator(diff_coeff));
    if (diffusion_function_ != nullptr) {
      K_->AddDomainIntegrator(new mfem::MassIntegrator(*diffusion_function_));
    }
    K_->Assemble();
    K_->FormSystemMatrix(ess_tdof_list_, Kmat_);
    // Compute T on the next ImplicitSolve
    delete T_;
    T_ = nullptr;
    // Don't repeat assembly again
    assembled_ = true;
  }
}

}  // namespace experimental
}  // namespace bdm

#endif  // USE_MFEM
