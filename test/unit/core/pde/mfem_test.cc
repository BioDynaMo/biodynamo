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
#include <chrono>
#include <numeric>
#include "biodynamo.h"
#include "core/agent/cell.h"
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

// We test if the SetODESolver member function works correctly. Especially, we
// test if the mapping between the enum MFEMODESolver and the member is correct.
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

// We test if the SetOperator and GetMolOperator member function work
// correctly. Moreover, we test if the mapping between the enum PDEOperator
// and the members are correct.
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

// Here, we test if the solver actually takes the correct time steps and
// simulates the right amount of time.
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

// We test if we correctly copy from a bdm Double3 to a mfem::Vector.
TEST(MethodOfLineTest, ConvertToMFEMVector) {
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

  // Test the conversion
  Double3 bdm{0.1, 0.2, 0.3};
  mfem::Vector mfem_vec = ConvertToMFEMVector(bdm);
  EXPECT_EQ(bdm.size(), mfem_vec.Size());
  EXPECT_EQ(bdm.Norm(), mfem_vec.Norml2());
  EXPECT_EQ(0.1, mfem_vec.Min());
  EXPECT_EQ(0.3, mfem_vec.Max());
}

// Here, we test if we can read of the solution of the grid at specific
// postions by providing the agent itself.
TEST(MethodOfLineTest, ContainedInElementOrNeighbour) {
  // Create a simple one element hex mesh
  mfem::Mesh mesh =
      mfem::Mesh::MakeCartesian3D(10, 10, 10, mfem::Element::Type::HEXAHEDRON);

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
  bdm_position[0] = 0.43291034;
  bdm_position[1] = 0.54829203;
  bdm_position[2] = 0.72717444;

  // Locate Agent
  auto res = solver.GetSolutionAtPosition(bdm_position);
  int fe_id = res.first;
  double grid_value = res.second;
  EXPECT_LT(abs(bdm_position.Norm() - grid_value), 0.1);

  // Test if contained in element
  mfem::IntegrationPoint ip;
  EXPECT_TRUE(solver.ContainedInElement(bdm_position, fe_id, ip));
  EXPECT_FALSE(solver.ContainedInElement(bdm_position, fe_id + 1, ip));
  EXPECT_FALSE(solver.ContainedInElement(bdm_position, fe_id - 1, ip));

  // Move to Neighboring boxes and see if we find the neighbour boxes
  for (int dimension = 0; dimension < 3; dimension++) {
    auto new_position_1 = bdm_position;
    auto new_position_2 = bdm_position;
    new_position_1[dimension] += 0.1;
    new_position_2[dimension] -= 0.1;
    auto res1 = solver.ContainedInNeighbors(new_position_1, fe_id, ip);
    auto res2 = solver.ContainedInNeighbors(new_position_2, fe_id, ip);
    EXPECT_NE(res1, fe_id);
    EXPECT_NE(res2, fe_id);
    EXPECT_NE(res1, res2);
    EXPECT_NE(res1, std::numeric_limits<int>::max());
    EXPECT_NE(res2, std::numeric_limits<int>::max());
    // move further and expect not to find it in neighbors
    new_position_1[dimension] += 0.1;
    new_position_2[dimension] -= 0.1;
    res1 = solver.ContainedInNeighbors(new_position_1, fe_id, ip);
    res2 = solver.ContainedInNeighbors(new_position_2, fe_id, ip);
    EXPECT_EQ(res1, std::numeric_limits<int>::max());
    EXPECT_EQ(res2, std::numeric_limits<int>::max());
  }
}

// Here, we test if we can read of the solution of the grid at specific
// postions by providing the postion directly.
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
  grid_value = solver.GetSolutionAtPosition(bdm_position).second;
  EXPECT_EQ(bdm_position.Norm(), grid_value);

  // Case  (On nodes by construction - after first refinement)
  bdm_position[0] = 0.75;
  bdm_position[1] = 0.6;
  bdm_position[2] = 0.85;
  grid_value = solver.GetSolutionAtPosition(bdm_position).second;
  EXPECT_EQ(bdm_position.Norm(), grid_value);

  // Case  (On nodes by construction - after second refinement)
  bdm_position[0] = 0.325;
  bdm_position[1] = 0.175;
  bdm_position[2] = 0.550;
  grid_value = solver.GetSolutionAtPosition(bdm_position).second;
  EXPECT_EQ(bdm_position.Norm(), grid_value);

  // Case  (Not on nodes by construction - after second refinement)
  bdm_position[0] = 0.43291034;
  bdm_position[1] = 0.54829203;
  bdm_position[2] = 0.92717444;
  grid_value = solver.GetSolutionAtPosition(bdm_position).second;
  EXPECT_NE(bdm_position.Norm(), grid_value);
  EXPECT_LT(abs(bdm_position.Norm() - grid_value), grid_value * 0.001);
}

// Here, we test if we can read of the solution of the grid at specific
// postions by providing the agent itself.
TEST(MethodOfLineTest, GetSolutionAtAgentPosition) {
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

  // Dummy simulation
  Simulation simulation("MethodOfLineTest-GetSolutionAtAgentPosition");
  auto* rm = simulation.GetResourceManager();
  auto* scheduler = simulation.GetScheduler();
  auto* cell = new Cell(1.0);
  rm->AddAgent(cell);
  scheduler->FinalizeInitialization();

  // Check the Function initialization at a few points
  Double3 bdm_position;
  double grid_value;

  // Check if finite element id of the agent is initalized correctly. After
  // checking the value at the agent position, the agents should remember the
  // element id & therefore this should change.
  EXPECT_EQ(cell->GetFiniteElementID(), std::numeric_limits<int>::max());

  // Case 1 (On nodes by construction)
  bdm_position[0] = 0.1;
  bdm_position[1] = 0.2;
  bdm_position[2] = 0.3;
  cell->SetPosition(bdm_position);
  grid_value = solver.GetSolutionAtAgentPosition(cell);
  EXPECT_EQ(bdm_position.Norm(), grid_value);
  EXPECT_NE(cell->GetFiniteElementID(), std::numeric_limits<int>::max());

  // Case  (On nodes by construction - after first refinement)
  bdm_position[0] = 0.75;
  bdm_position[1] = 0.6;
  bdm_position[2] = 0.85;
  cell->SetPosition(bdm_position);
  grid_value = solver.GetSolutionAtAgentPosition(cell);
  EXPECT_EQ(bdm_position.Norm(), grid_value);
  EXPECT_NE(cell->GetFiniteElementID(), std::numeric_limits<int>::max());

  // Case  (On nodes by construction - after second refinement)
  bdm_position[0] = 0.325;
  bdm_position[1] = 0.175;
  bdm_position[2] = 0.550;
  cell->SetPosition(bdm_position);
  grid_value = solver.GetSolutionAtAgentPosition(cell);
  EXPECT_EQ(bdm_position.Norm(), grid_value);
  EXPECT_NE(cell->GetFiniteElementID(), std::numeric_limits<int>::max());

  // Case  (Not on nodes by construction - after second refinement)
  bdm_position[0] = 0.43291034;
  bdm_position[1] = 0.54829203;
  bdm_position[2] = 0.92717444;
  cell->SetPosition(bdm_position);
  grid_value = solver.GetSolutionAtAgentPosition(cell);
  EXPECT_NE(bdm_position.Norm(), grid_value);
  EXPECT_LT(abs(bdm_position.Norm() - grid_value), grid_value * 0.001);
  EXPECT_NE(cell->GetFiniteElementID(), std::numeric_limits<int>::max());
}

// Here, we test if we can read of the solution of the grid at specific
// postions by providing the agent itself. The main reason for this test is to
// see if our accelerated search (first previous element, then neighbors, then
// checking all elements) accelerates the application as expected.
TEST(MethodOfLineTest, StepByStepLocalization) {
  // Create a simple one element hex mesh
  mfem::Mesh mesh = mfem::Mesh::MakeCartesian3D(
      100, 100, 100, mfem::Element::Type::HEXAHEDRON);

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

  // Dummy simulation
  int num_agents = 10;
  Simulation simulation("MethodOfLineTest-GetSolutionAtAgentPosition");
  auto* rm = simulation.GetResourceManager();
  auto* scheduler = simulation.GetScheduler();
  auto* random = simulation.GetRandom();
  for (int i = 0; i < num_agents; i++) {
    auto* cell = new Cell(1.0);
    // don't go all the way to 1.0 because of case 3.
    auto pos = random->UniformArray<3>(0.98);
    cell->SetPosition(pos);
    rm->AddAgent(cell);
  }
  scheduler->FinalizeInitialization();

  // Check the Function initialization at a few points. For the specific setup
  // of element size, type, and the InitialGridValues, the values within one
  // element vary at most abs_error. We therefore execpt results that are
  // within this tolerance.
  double abs_error = 0.01;

  // Check if finite element id of the agent is initalized correctly. After
  // checking the value at the agent position, the agents should remember the
  // element id & therefore this should change.
  rm->ForEachAgent([&](Agent* agent) {
    EXPECT_EQ(agent->GetFiniteElementID(), std::numeric_limits<int>::max());
  });

  // Case 1 (Locate agents with linear search in mesh)
  std::chrono::steady_clock::time_point begin_linear_search =
      std::chrono::steady_clock::now();
  rm->ForEachAgent([&](Agent* agent) {
    double grid_value = solver.GetSolutionAtAgentPosition(agent);
    EXPECT_LT(abs(agent->GetPosition().Norm() - grid_value), abs_error);
  });
  std::chrono::steady_clock::time_point end_linear_search =
      std::chrono::steady_clock::now();
  rm->ForEachAgent([&](Agent* agent) {
    EXPECT_NE(agent->GetFiniteElementID(), std::numeric_limits<int>::max());
  });

  // Case 2 (Repeat exercise, this time agents should already know their
  // position and the search should be significantly faster.)
  mfem::IntegrationPoint dummy;
  rm->ForEachAgent([&](Agent* agent) {
    EXPECT_EQ(true,
              solver.ContainedInElement(agent->GetPosition(),
                                        agent->GetFiniteElementID(), dummy));
  });
  std::chrono::steady_clock::time_point begin_no_search =
      std::chrono::steady_clock::now();
  rm->ForEachAgent([&](Agent* agent) {
    double grid_value = solver.GetSolutionAtAgentPosition(agent);
    EXPECT_LT(abs(agent->GetPosition().Norm() - grid_value), abs_error);
  });
  std::chrono::steady_clock::time_point end_no_search =
      std::chrono::steady_clock::now();

  // Case 3 (Repeat exercise, this time agents should already know their
  // position but we move it into a neighbor element. The search should be
  // slightly slower than in case 2 but still significantly faster than in
  // case 1)
  rm->ForEachAgent([&](Agent* agent) {
    agent->SetPosition(agent->GetPosition() + random->UniformArray<3>(0.01));
  });
  solver.UpdateElementToVertexTable();
  std::chrono::steady_clock::time_point begin_neighbor_search =
      std::chrono::steady_clock::now();
  rm->ForEachAgent([&](Agent* agent) {
    double grid_value = solver.GetSolutionAtAgentPosition(agent);
    EXPECT_LT(abs(agent->GetPosition().Norm() - grid_value), abs_error);
  });
  std::chrono::steady_clock::time_point end_neighbor_search =
      std::chrono::steady_clock::now();

  // Compute search times
  auto duration_linear_search =
      std::chrono::duration_cast<std::chrono::nanoseconds>(end_linear_search -
                                                           begin_linear_search)
          .count();
  auto duration_no_search =
      std::chrono::duration_cast<std::chrono::nanoseconds>(end_no_search -
                                                           begin_no_search)
          .count();
  auto duration_neighbor_search =
      std::chrono::duration_cast<std::chrono::nanoseconds>(
          end_neighbor_search - begin_neighbor_search)
          .count();

  // Test if our search acceleration matches our expectation.
  EXPECT_LT(duration_no_search, duration_neighbor_search);
  EXPECT_LT(duration_no_search, duration_linear_search);
  EXPECT_LT(duration_neighbor_search, duration_linear_search);
}

// Test the integration of MFEM into BioDynaMo via the Model Initializer and the
// Resource Manager.
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

  // Test if all pointers (mesh and solver) are not nullptr and we have a
  // unique address for each of them.
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
