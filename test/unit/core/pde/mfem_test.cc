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
#include "biodynamo.h"
#include "core/pde/mfem_mol.h"

#define TEST_NAME typeid(*this).name()

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

enum Substances { kSubstance1, kSubstance2, kSubstance3 };

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
  }
}

TEST(MethodOfLineTest, GetSolutionAtPosition) {
  // Create a simple one element hex mesh
  mfem::Mesh mesh =
      mfem::Mesh::MakeCartesian3D(10, 10, 10, mfem::Element::Type::HEXAHEDRON);
  mesh.UniformRefinement();
  mesh.UniformRefinement();

  // Define function to set inital values of the Mesh
  auto InitializeGridValues = [&](const mfem::Vector& x) { return x.Norml2(); };

  // Define numeric parameters
  std::vector<double> parameters{0.1};

  // Define empty functions vector for constructor
  std::vector<std::function<double(const mfem::Vector&)>> operator_functions;

  // Create MethodOfLineSolver
  MethodOfLineSolver solver(&mesh, 1, 3, MFEMODESolver::kBackwardEulerSolver,
                            PDEOperator::kDiffusion, InitializeGridValues,
                            parameters, operator_functions);

  // Check the Function initialization at a few points
  Double3 bdm_position;
  double grid_value;

  // Case 1 (On nodes by construction)
  bdm_position[0] = 0.1;
  bdm_position[1] = 0.2;
  bdm_position[2] = 0.3;
  grid_value = solver.GetSolutionAtPosition(bdm_position);
  EXPECT_EQ(bdm_position.Norm(), grid_value);

  // Case  (On nodes by construction - after first refinement)
  bdm_position[0] = 0.75;
  bdm_position[1] = 0.6;
  bdm_position[2] = 0.85;
  grid_value = solver.GetSolutionAtPosition(bdm_position);
  EXPECT_EQ(bdm_position.Norm(), grid_value);

  // Case  (On nodes by construction - after second refinement)
  bdm_position[0] = 0.325;
  bdm_position[1] = 0.175;
  bdm_position[2] = 0.550;
  grid_value = solver.GetSolutionAtPosition(bdm_position);
  EXPECT_EQ(bdm_position.Norm(), grid_value);

  // Case  (Not on nodes by construction - after second refinement)
  bdm_position[0] = 0.43291034;
  bdm_position[1] = 0.54829203;
  bdm_position[2] = 0.92717444;
  grid_value = solver.GetSolutionAtPosition(bdm_position);
  EXPECT_NE(bdm_position.Norm(), grid_value);
  EXPECT_LT(abs(bdm_position.Norm() - grid_value), grid_value * 0.001);
}

TEST(MFEMIntegration, ModelInitializerAndRessourceManagerTest) {
  auto set_param = [](auto* param) {
    param->bound_space = Param::BoundSpaceMode::kClosed;
    param->min_bound = 0;
    param->max_bound = 250;
    param->calculate_gradients = false;
  };
  Simulation simulation(TEST_NAME, set_param);

  auto* rm = simulation.GetResourceManager();
  auto* param = simulation.GetParam();

  // Create one cell at a random position
  auto construct = [](const Double3& position) {
    Cell* cell = new Cell(position);
    cell->SetDiameter(10);
    return cell;
  };
  ModelInitializer::CreateAgentsRandom(param->min_bound, param->max_bound, 1,
                                       construct);

  // Create a simple one element hex mesh
  mfem::Mesh* mesh = new mfem::Mesh();
  *mesh =
      mfem::Mesh::MakeCartesian3D(10, 10, 10, mfem::Element::Type::HEXAHEDRON);
  mesh->UniformRefinement();
  mesh->UniformRefinement();

  // Define function to set inital values of the Mesh
  auto InitializeGridValues = [&](const mfem::Vector& x) { return x.Norml2(); };

  // Define numeric parameters
  std::vector<double> parameters{0.1};

  // Define empty functions vector for constructor
  std::vector<std::function<double(const mfem::Vector&)>> operator_functions;

  // Define the first substances in our simulation
  ModelInitializer::DefineMFEMSubstanceOnMesh(
      mesh, kSubstance1, "kSubstance1", 1, 3,
      MFEMODESolver::kBackwardEulerSolver, PDEOperator::kDiffusion,
      InitializeGridValues, parameters, operator_functions);

  // Define the second substances in our simulation
  ModelInitializer::DefineMFEMSubstanceAndMesh(
      10, 10, 10, 1.0, 1.0, 1.0, mfem::Element::TETRAHEDRON, kSubstance2,
      "kSubstance2", 1, 3, MFEMODESolver::kBackwardEulerSolver,
      PDEOperator::kDiffusion, InitializeGridValues, parameters,
      operator_functions);

  // Define the third substances in our simulation
  ModelInitializer::DefineMFEMSubstanceAndMesh(
      15, 15, 15, 1.3, 1.3, 1.3, mfem::Element::TETRAHEDRON, kSubstance3,
      "kSubstance3", 2, 3, MFEMODESolver::kBackwardEulerSolver,
      PDEOperator::kDiffusion, InitializeGridValues, parameters,
      operator_functions);

  simulation.GetEnvironment()->Update();

  // Test if we have 3 registered MFEM Meshes
  EXPECT_EQ(rm->GetNumMFEMMeshes(), 3);

  // Get registered meshes and solvers
  auto* mesh1 = rm->GetMFEMGrid(0).first;
  auto* mesh2 = rm->GetMFEMGrid(1).first;
  auto* mesh3 = rm->GetMFEMGrid(2).first;
  auto* solver1 = rm->GetMFEMGrid(0).second;
  auto* solver2 = rm->GetMFEMGrid(1).second;
  auto* solver3 = rm->GetMFEMGrid(2).second;

  // Test if all pointers (mesh and solver) are not nullptr and we have a unique
  // address for each of them.
  EXPECT_NE(nullptr, mesh1);
  EXPECT_NE(nullptr, mesh2);
  EXPECT_NE(nullptr, mesh3);
  EXPECT_NE(nullptr, solver1);
  EXPECT_NE(nullptr, solver2);
  EXPECT_NE(nullptr, solver3);
  EXPECT_NE(mesh1, mesh2);
  EXPECT_NE(mesh1, mesh3);
  EXPECT_NE(mesh2, mesh3);
  EXPECT_NE(solver1, solver2);
  EXPECT_NE(solver1, solver3);
  EXPECT_NE(solver2, solver3);

  // Get the same pointers but from string search
  auto* mesh_1 = rm->GetMFEMGrid("kSubstance1").first;
  auto* mesh_2 = rm->GetMFEMGrid("kSubstance2").first;
  auto* mesh_3 = rm->GetMFEMGrid("kSubstance3").first;
  auto* solver_1 = rm->GetMFEMGrid("kSubstance1").second;
  auto* solver_2 = rm->GetMFEMGrid("kSubstance2").second;
  auto* solver_3 = rm->GetMFEMGrid("kSubstance3").second;

  // Test if string and id search result in the same references
  EXPECT_EQ(mesh1, mesh_1);
  EXPECT_EQ(mesh2, mesh_2);
  EXPECT_EQ(mesh3, mesh_3);
  EXPECT_EQ(solver1, solver_1);
  EXPECT_EQ(solver2, solver_2);
  EXPECT_EQ(solver3, solver_3);

  // Test scheduled default operation. Implicit test ForEachMFEMGrid.
  auto* scheduler = simulation.GetScheduler();
  scheduler->Simulate(2);
  EXPECT_EQ(0.01, solver1->GetSimTime());
  scheduler->Simulate(1);
  EXPECT_EQ(0.02, solver2->GetSimTime());
  scheduler->Simulate(1);
  EXPECT_EQ(0.03, solver3->GetSimTime());

  // Remove grids
  rm->RemoveMFEMMesh(0);
  EXPECT_EQ(rm->GetNumMFEMMeshes(), 2);
  rm->RemoveMFEMMesh(1);
  EXPECT_EQ(rm->GetNumMFEMMeshes(), 1);
  rm->RemoveMFEMMesh(2);
  EXPECT_EQ(rm->GetNumMFEMMeshes(), 0);
}

}  // namespace experimental
}  // namespace bdm

#endif  // USE_MFEM
