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

#include <gtest/gtest.h>
#include <numeric>
#include "core/pde/mfem_mol.h"

namespace bdm {
namespace experimental {

mfem::Mesh* CreateMesh() {
  int dim = 3, nv = 8, ne = 1, nb = 0, sdim = 3;
  mfem::Mesh* mesh = new mfem::Mesh(dim, nv, ne, nb, sdim);
  mesh->AddVertex(mfem::Vertex(0., 0., 0.)());
  mesh->AddVertex(mfem::Vertex(1., 0., 0.)());
  mesh->AddVertex(mfem::Vertex(1., 1., 0.)());
  mesh->AddVertex(mfem::Vertex(0., 1., 0.)());
  mesh->AddVertex(mfem::Vertex(0., 0., 1.)());
  mesh->AddVertex(mfem::Vertex(1., 0., 1.)());
  mesh->AddVertex(mfem::Vertex(1., 1., 1.)());
  mesh->AddVertex(mfem::Vertex(0., 1., 1.)());
  return mesh;
}

struct TimeStepGenerator {
  double current;
  double increase;
  TimeStepGenerator(double time_step) {
    current = 0.00;
    increase = time_step;
  }
  double operator()() {
    current += increase;
    return current;
  }
};

TEST(MethodOfLineTest, SetODESolver) {
  // Create a simple one element hex mesh
  mfem::Mesh* mesh = CreateMesh();

  // Define function to set inital values of the Mesh
  auto InitializeGridValues = [&](const mfem::Vector& x) { return 0.0; };

  // Define numeric parameters
  std::vector<double> parameters{0.1};

  // Define empty functions vector for constructor
  std::vector<std::function<double(const mfem::Vector&)>> operator_functions;

  // Create MethodOfLineSolver
  MethodOfLineSolver solver(mesh, 1, 3, MFEMODESolver::kBackwardEulerSolver,
                            PDEOperator::kDiffusion, InitializeGridValues,
                            parameters, operator_functions);

  std::string solver_name;
  auto* tmp = solver.GetODESolver();
  solver_name = typeid(*tmp).name();
  EXPECT_TRUE(solver_name.find("BackwardEuler") != std::string::npos);
  solver.SetODESolver(MFEMODESolver::kBackwardEulerSolver);
  tmp = solver.GetODESolver();
  solver_name = typeid(*tmp).name();
  EXPECT_TRUE(solver_name.find("BackwardEulerSolver") != std::string::npos);
  solver.SetODESolver(MFEMODESolver::kSDIRK23Solver2);
  tmp = solver.GetODESolver();
  solver_name = typeid(*tmp).name();
  EXPECT_TRUE(solver_name.find("SDIRK23Solver") != std::string::npos);
  solver.SetODESolver(MFEMODESolver::kSDIRK33Solver);
  tmp = solver.GetODESolver();
  solver_name = typeid(*tmp).name();
  EXPECT_TRUE(solver_name.find("SDIRK33Solver") != std::string::npos);
  solver.SetODESolver(MFEMODESolver::kForwardEulerSolver);
  tmp = solver.GetODESolver();
  solver_name = typeid(*tmp).name();
  EXPECT_TRUE(solver_name.find("ForwardEulerSolver") != std::string::npos);
  solver.SetODESolver(MFEMODESolver::kRK2Solver);
  tmp = solver.GetODESolver();
  solver_name = typeid(*tmp).name();
  EXPECT_TRUE(solver_name.find("RK2Solver") != std::string::npos);
  solver.SetODESolver(MFEMODESolver::kRK3SSPSolver);
  tmp = solver.GetODESolver();
  solver_name = typeid(*tmp).name();
  EXPECT_TRUE(solver_name.find("RK3SSPSolver") != std::string::npos);
  solver.SetODESolver(MFEMODESolver::kRK4Solver);
  tmp = solver.GetODESolver();
  solver_name = typeid(*tmp).name();
  EXPECT_TRUE(solver_name.find("RK4Solver") != std::string::npos);
  solver.SetODESolver(MFEMODESolver::kGeneralizedAlphaSolver);
  tmp = solver.GetODESolver();
  solver_name = typeid(*tmp).name();
  EXPECT_TRUE(solver_name.find("GeneralizedAlphaSolver") != std::string::npos);
  solver.SetODESolver(MFEMODESolver::kImplicitMidpointSolver);
  tmp = solver.GetODESolver();
  solver_name = typeid(*tmp).name();
  EXPECT_TRUE(solver_name.find("ImplicitMidpointSolver") != std::string::npos);
  solver.SetODESolver(MFEMODESolver::kSDIRK23Solver1);
  tmp = solver.GetODESolver();
  solver_name = typeid(*tmp).name();
  EXPECT_TRUE(solver_name.find("SDIRK23Solver") != std::string::npos);
  solver.SetODESolver(MFEMODESolver::kSDIRK34Solver);
  tmp = solver.GetODESolver();
  solver_name = typeid(*tmp).name();
  EXPECT_TRUE(solver_name.find("SDIRK34Solver") != std::string::npos);
  delete mesh;
}

TEST(MethodOfLineTest, SetOperator) {
  // Create a simple one element hex mesh
  mfem::Mesh* mesh = CreateMesh();

  // Define function to set inital values of the Mesh
  auto InitializeGridValues = [&](const mfem::Vector& x) { return 0.0; };

  // Define numeric parameters
  std::vector<double> parameters{0.1, 0.2};

  // Define empty functions vector for constructor
  std::vector<std::function<double(const mfem::Vector&)>> operator_functions;
  operator_functions.push_back(InitializeGridValues);

  // Create MethodOfLineSolver
  MethodOfLineSolver solver(mesh, 1, 3, MFEMODESolver::kBackwardEulerSolver,
                            PDEOperator::kDiffusion, InitializeGridValues,
                            parameters, operator_functions);

  std::string operator_name;
  auto* tmp = solver.GetMolOperator();
  operator_name = typeid(*tmp).name();
  EXPECT_TRUE(operator_name.find("DiffusionOperator") != std::string::npos);
  solver.SetOperator(PDEOperator::kDiffusionWithFunction);
  tmp = solver.GetMolOperator();
  operator_name = typeid(*tmp).name();
  EXPECT_TRUE(operator_name.find("DiffusionOperator") != std::string::npos);
  solver.SetOperator(PDEOperator::kConduction);
  tmp = solver.GetMolOperator();
  operator_name = typeid(*tmp).name();
  EXPECT_TRUE(operator_name.find("ConductionOperator") != std::string::npos);
  delete mesh;
}

TEST(MethodOfLineTest, Step) {
  // Create a simple one element hex mesh
  mfem::Mesh mesh =
      mfem::Mesh::MakeCartesian3D(10, 10, 10, mfem::Element::Type::TETRAHEDRON);

  // Define function to set inital values of the Mesh
  auto InitializeGridValues = [&](const mfem::Vector& x) { return x[0]; };

  // Define numeric parameters
  std::vector<double> parameters{0.1};

  // Define empty functions vector for constructor
  std::vector<std::function<double(const mfem::Vector&)>> operator_functions;

  // Create MethodOfLineSolver
  MethodOfLineSolver solver(&mesh, 1, 3, MFEMODESolver::kBackwardEulerSolver,
                            PDEOperator::kDiffusion, InitializeGridValues,
                            parameters, operator_functions);

  // Define time steps
  size_t num_steps{200};
  double time_step{0.01};
  TimeStepGenerator gen = TimeStepGenerator(time_step);
  std::vector<double> time_steps(num_steps);
  std::vector<double> sim_time(num_steps);
  std::generate(time_steps.begin(), time_steps.end(), gen);
  std::partial_sum(time_steps.begin(), time_steps.end(), sim_time.begin());

  // Check if steps are simulated correctly.
  for (size_t i = 0; i < num_steps; i++) {
    solver.Step(time_steps[i]);
    EXPECT_EQ(sim_time[i], solver.GetSimTime());
    std::cout << time_steps[i] << " " << sim_time[i] << " "
              << solver.GetSimTime() << std::endl;
  }
}

}  // namespace experimental
}  // namespace bdm

#endif  // USE_MFEM
