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

#include "core/model_initializer.h"
#include "core/diffusion/diffusion_grid.h"
#include "core/diffusion/euler_grid.h"
#include "core/diffusion/runge_kutta_grid.h"

namespace bdm {

void ModelInitializer::DefineSubstance(size_t substance_id,
                                       const std::string& substance_name,
                                       double diffusion_coeff,
                                       double decay_constant, int resolution) {
  auto* sim = Simulation::GetActive();
  auto* param = sim->GetParam();
  auto* rm = sim->GetResourceManager();
  DiffusionGrid* dgrid = nullptr;
  if (param->diffusion_method == "euler") {
    dgrid = new EulerGrid(substance_id, substance_name, diffusion_coeff,
                          decay_constant, resolution);
  } else if (param->diffusion_method == "runge-kutta") {
    dgrid = new RungeKuttaGrid(substance_id, substance_name, diffusion_coeff,
                               decay_constant, resolution);
  } else {
    Log::Error("ModelInitializer::DefineSubstance", "Diffusion method '",
               param->diffusion_method,
               "' does not exist. Defaulting to 'euler'");
    dgrid = new EulerGrid(substance_id, substance_name, diffusion_coeff,
                          decay_constant, resolution);
  }

  rm->AddDiffusionGrid(dgrid);
}

#ifdef USE_MFEM

void ModelInitializer::DefineMFEMSubstanceOnMesh(
    mfem::Mesh* mesh, size_t substance_id, const std::string& substance_name,
    int order, int dimension, bdm::experimental::MFEMODESolver ode_solver_id,
    bdm::experimental::PDEOperator pde_oper_id,
    std::function<double(const mfem::Vector&)> InitialGridValues,
    std::vector<double> numeric_operator_parameters,
    std::vector<std::function<double(const mfem::Vector&)>>
        operator_functions) {
  auto* sim = Simulation::GetActive();
  auto* rm = sim->GetResourceManager();
  // ToDo(tobias): can this be conde without new?
  auto* solver = new bdm::experimental::TimeDependentScalarField3d(
      mesh, order, dimension, ode_solver_id, pde_oper_id, InitialGridValues,
      numeric_operator_parameters, operator_functions);
  solver->SetSubstanceId(substance_id);
  solver->SetSubstanceName(substance_name);
  auto mfem_grid = std::make_pair(mesh, solver);
  rm->AddMFEMMesh(mfem_grid);
}

void ModelInitializer::DefineMFEMSubstanceAndMesh(
    int resolution_x, int resolution_y, int resolution_z, double x_max,
    double y_max, double z_max, mfem::Element::Type element_type,
    size_t substance_id, const std::string& substance_name, int order,
    int dimension, bdm::experimental::MFEMODESolver ode_solver_id,
    bdm::experimental::PDEOperator pde_oper_id,
    std::function<double(const mfem::Vector&)> InitialGridValues,
    std::vector<double> numeric_operator_parameters,
    std::vector<std::function<double(const mfem::Vector&)>>
        operator_functions) {
  mfem::Mesh* mesh = new mfem::Mesh();
  *mesh = mfem::Mesh::MakeCartesian3D(resolution_x, resolution_y, resolution_z,
                                      element_type, x_max, y_max, z_max);
  DefineMFEMSubstanceOnMesh(mesh, substance_id, substance_name, order,
                            dimension, ode_solver_id, pde_oper_id,
                            InitialGridValues, numeric_operator_parameters,
                            operator_functions);
}

#endif

}  // namespace bdm
