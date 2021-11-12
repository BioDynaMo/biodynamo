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

#include <chrono>
#include <numeric>
#include "biodynamo.h"
#include "core/agent/cell.h"
#include "core/pde/mfem_mol.h"
#include "gtest/gtest.h"

#define TEST_NAME typeid(*this).name()

namespace bdm {
namespace experimental {

enum Substances { kSubstance1, kSubstance2, kSubstance3 };

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

TimeDependentScalarField3d* InitializeScalarField(
    size_t num_mesh_refinements = 0) {
  // Create a simple
  mfem::Mesh* mesh = new mfem::Mesh();
  *mesh =
      mfem::Mesh::MakeCartesian3D(10, 10, 10, mfem::Element::Type::TETRAHEDRON);
  for (size_t i = 0; i < num_mesh_refinements; i++) {
    mesh->UniformRefinement();
  }
  // Define function to set inital values of the Mesh
  auto InitializeGridValues = [&](const mfem::Vector& x) { return x.Norml2(); };

  // Define numeric parameters
  std::vector<double> parameters{0.1, 0.2};

  // Define functions vector for constructor
  std::vector<std::function<double(const mfem::Vector&)>> operator_functions;
  operator_functions.push_back(InitializeGridValues);

  // Create TimeDependentScalarField3d
  TimeDependentScalarField3d* scalar_field = new TimeDependentScalarField3d(
      mesh, 1, 3, MFEMODESolver::kBackwardEulerSolver, PDEOperator::kDiffusion,
      InitializeGridValues, parameters, operator_functions);

  return scalar_field;
}

// Function to set up a simulation and a TimeDependentScalarField3d.
inline void InitializeSimulation(std::string sim_name,
                                 std::function<void(Param*)> set_param) {
  Simulation simulation(sim_name, set_param);

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
      mfem::Mesh::MakeCartesian3D(10, 10, 10, mfem::Element::Type::TETRAHEDRON);

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
}

// Tests if scalar_field retrieves the correct values at bdm_position via
// GetSolutionAtAgentPosition and GetSolutionAtPosition.
void TestGridValues(TimeDependentScalarField3d* scalar_field,
                    Double3& bdm_position, Cell* cell,
                    double tol = std::numeric_limits<double>::max()) {
  cell->SetPosition(bdm_position);
  double grid_value_from_agent = scalar_field->GetSolutionAtAgentPosition(cell);
  double grid_value_from_position =
      scalar_field->GetSolutionAtPosition(bdm_position).second;
  double computed_fe_index_from_position =
      scalar_field->GetSolutionAtPosition(bdm_position).first;
  double computed_fe_index_from_agent = cell->GetFiniteElementID();
  if (tol == std::numeric_limits<double>::max()) {
    EXPECT_DOUBLE_EQ(bdm_position.Norm(), grid_value_from_position);
    EXPECT_DOUBLE_EQ(bdm_position.Norm(), grid_value_from_agent);
  } else {
    EXPECT_NE(bdm_position.Norm(), grid_value_from_position);
    EXPECT_LT(abs(bdm_position.Norm() - grid_value_from_position),
              grid_value_from_position * tol);
    EXPECT_NE(bdm_position.Norm(), grid_value_from_agent);
    EXPECT_LT(abs(bdm_position.Norm() - grid_value_from_agent),
              grid_value_from_agent * tol);
  }
  EXPECT_NE(std::numeric_limits<int>::max(), computed_fe_index_from_position);
  EXPECT_NE(std::numeric_limits<int>::max(), computed_fe_index_from_agent);
}

// Get the current value of steady clock from chrono library.
std::chrono::steady_clock::time_point GetChronoTime() {
  return std::chrono::steady_clock::now();
}

// Compute the duration that passed between two time points in
// nanoseconds.
int64_t ComputeDuration(std::chrono::steady_clock::time_point& end,
                        std::chrono::steady_clock::time_point& start) {
  return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start)
      .count();
}

// We test if the SetODESolver member function works correctly. Especially, we
// test if the mapping between the enum MFEMODESolver and the member is correct.
TEST(TimeDependentScalarField3dTest, SetODESolver) {
  auto* scalar_field = InitializeScalarField();

  std::string scalar_field_name;
  auto* tmp = scalar_field->GetODESolver();
  scalar_field_name = typeid(*tmp).name();
  EXPECT_TRUE(scalar_field_name.find("BackwardEuler") != std::string::npos);
  scalar_field->SetODESolver(MFEMODESolver::kBackwardEulerSolver);
  tmp = scalar_field->GetODESolver();
  scalar_field_name = typeid(*tmp).name();
  EXPECT_TRUE(scalar_field_name.find("BackwardEulerSolver") !=
              std::string::npos);
  scalar_field->SetODESolver(MFEMODESolver::kSDIRK23Solver2);
  tmp = scalar_field->GetODESolver();
  scalar_field_name = typeid(*tmp).name();
  EXPECT_TRUE(scalar_field_name.find("SDIRK23Solver") != std::string::npos);
  scalar_field->SetODESolver(MFEMODESolver::kSDIRK33Solver);
  tmp = scalar_field->GetODESolver();
  scalar_field_name = typeid(*tmp).name();
  EXPECT_TRUE(scalar_field_name.find("SDIRK33Solver") != std::string::npos);
  scalar_field->SetODESolver(MFEMODESolver::kForwardEulerSolver);
  tmp = scalar_field->GetODESolver();
  scalar_field_name = typeid(*tmp).name();
  EXPECT_TRUE(scalar_field_name.find("ForwardEulerSolver") !=
              std::string::npos);
  scalar_field->SetODESolver(MFEMODESolver::kRK2Solver);
  tmp = scalar_field->GetODESolver();
  scalar_field_name = typeid(*tmp).name();
  EXPECT_TRUE(scalar_field_name.find("RK2Solver") != std::string::npos);
  scalar_field->SetODESolver(MFEMODESolver::kRK3SSPSolver);
  tmp = scalar_field->GetODESolver();
  scalar_field_name = typeid(*tmp).name();
  EXPECT_TRUE(scalar_field_name.find("RK3SSPSolver") != std::string::npos);
  scalar_field->SetODESolver(MFEMODESolver::kRK4Solver);
  tmp = scalar_field->GetODESolver();
  scalar_field_name = typeid(*tmp).name();
  EXPECT_TRUE(scalar_field_name.find("RK4Solver") != std::string::npos);
  scalar_field->SetODESolver(MFEMODESolver::kGeneralizedAlphaSolver);
  tmp = scalar_field->GetODESolver();
  scalar_field_name = typeid(*tmp).name();
  EXPECT_TRUE(scalar_field_name.find("GeneralizedAlphaSolver") !=
              std::string::npos);
  scalar_field->SetODESolver(MFEMODESolver::kImplicitMidpointSolver);
  tmp = scalar_field->GetODESolver();
  scalar_field_name = typeid(*tmp).name();
  EXPECT_TRUE(scalar_field_name.find("ImplicitMidpointSolver") !=
              std::string::npos);
  scalar_field->SetODESolver(MFEMODESolver::kSDIRK23Solver1);
  tmp = scalar_field->GetODESolver();
  scalar_field_name = typeid(*tmp).name();
  EXPECT_TRUE(scalar_field_name.find("SDIRK23Solver") != std::string::npos);
  scalar_field->SetODESolver(MFEMODESolver::kSDIRK34Solver);
  tmp = scalar_field->GetODESolver();
  scalar_field_name = typeid(*tmp).name();
  EXPECT_TRUE(scalar_field_name.find("SDIRK34Solver") != std::string::npos);
  delete scalar_field->GetMesh();
  delete scalar_field;
}

// We test if the SetOperator and GetMolOperator member function work
// correctly. Moreover, we test if the mapping between the enum PDEOperator
// and the members are correct.
TEST(TimeDependentScalarField3dTest, SetOperator) {
  auto* scalar_field = InitializeScalarField();

  std::string operator_name;
  auto* tmp = scalar_field->GetMolOperator();
  operator_name = typeid(*tmp).name();
  EXPECT_TRUE(operator_name.find("DiffusionOperator") != std::string::npos);
  scalar_field->SetOperator(PDEOperator::kDiffusionWithFunction);
  tmp = scalar_field->GetMolOperator();
  operator_name = typeid(*tmp).name();
  EXPECT_TRUE(operator_name.find("DiffusionOperator") != std::string::npos);
  scalar_field->SetOperator(PDEOperator::kConduction);
  tmp = scalar_field->GetMolOperator();
  operator_name = typeid(*tmp).name();
  EXPECT_TRUE(operator_name.find("ConductionOperator") != std::string::npos);
  delete scalar_field->GetMesh();
  delete scalar_field;
}

// Here, we test if the scalar_field actually takes the correct time steps and
// simulates the right amount of time.
TEST(TimeDependentScalarField3dTest, Step) {
  auto* scalar_field = InitializeScalarField();

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
    scalar_field->Step(time_steps[i]);
    EXPECT_EQ(sim_time[i], scalar_field->GetSimTime());
  }
  delete scalar_field->GetMesh();
  delete scalar_field;
}

// We test if we correctly copy from a bdm Double3 to a mfem::Vector.
TEST(TimeDependentScalarField3dTest, ConvertToMFEMVector) {
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
TEST(TimeDependentScalarField3dTest, ContainedInElementOrNeighbour) {
  auto* scalar_field = InitializeScalarField();

  // Check the Function initialization at a few points
  Double3 bdm_position;
  bdm_position[0] = 0.43291034;
  bdm_position[1] = 0.54829203;
  bdm_position[2] = 0.72717444;

  // Locate Agent
  auto res = scalar_field->GetSolutionAtPosition(bdm_position);
  int fe_id = res.first;
  double grid_value = res.second;
  EXPECT_LT(abs(bdm_position.Norm() - grid_value), 0.1);

  // Test if contained in element
  mfem::IntegrationPoint ip;
  EXPECT_TRUE(scalar_field->ContainedInElement(bdm_position, fe_id, ip));
  EXPECT_FALSE(scalar_field->ContainedInElement(bdm_position, fe_id + 1, ip));
  EXPECT_FALSE(scalar_field->ContainedInElement(bdm_position, fe_id - 1, ip));

  // Move to Neighboring boxes and see if we find the neighbour boxes
  for (int dimension = 0; dimension < 3; dimension++) {
    auto new_position_1 = bdm_position;
    auto new_position_2 = bdm_position;
    new_position_1[dimension] += 0.1;
    new_position_2[dimension] -= 0.1;
    auto res1 = scalar_field->ContainedInNeighbors(new_position_1, fe_id, ip);
    auto res2 = scalar_field->ContainedInNeighbors(new_position_2, fe_id, ip);
    EXPECT_NE(res1, fe_id);
    EXPECT_NE(res2, fe_id);
    EXPECT_NE(res1, res2);
    EXPECT_NE(res1, std::numeric_limits<int>::max());
    EXPECT_NE(res2, std::numeric_limits<int>::max());
    // move further and expect not to find it in neighbors
    new_position_1[dimension] += 0.1;
    new_position_2[dimension] -= 0.1;
    res1 = scalar_field->ContainedInNeighbors(new_position_1, fe_id, ip);
    res2 = scalar_field->ContainedInNeighbors(new_position_2, fe_id, ip);
    EXPECT_EQ(res1, std::numeric_limits<int>::max());
    EXPECT_EQ(res2, std::numeric_limits<int>::max());
  }

  delete scalar_field->GetMesh();
  delete scalar_field;
}

// Here, we test if we can read of the solution of the grid at specific
// postions by providing the agent or the position.
TEST(TimeDependentScalarField3dTest, GetSolutionAtAgentAndPosition) {
  auto* scalar_field = InitializeScalarField(2);

  // Dummy simulation
  Simulation simulation(
      "TimeDependentScalarField3dTest-GetSolutionAtAgentPosition");
  auto* cell = new Cell(1.0);

  // Check the Function initialization at a few points
  Double3 bdm_position;

  // Check if finite element id of the agent is initalized correctly. After
  // checking the value at the agent position, the agents should remember the
  // element id & therefore this should change.
  EXPECT_EQ(cell->GetFiniteElementID(), std::numeric_limits<int>::max());

  // Case 1 (On nodes by construction)
  bdm_position[0] = 0.1;
  bdm_position[1] = 0.2;
  bdm_position[2] = 0.3;
  TestGridValues(scalar_field, bdm_position, cell);

  // Case  (On nodes by construction - after first refinement)
  bdm_position[0] = 0.75;
  bdm_position[1] = 0.6;
  bdm_position[2] = 0.85;
  TestGridValues(scalar_field, bdm_position, cell);

  // Case  (On nodes by construction - after second refinement)
  bdm_position[0] = 0.325;
  bdm_position[1] = 0.175;
  bdm_position[2] = 0.550;
  TestGridValues(scalar_field, bdm_position, cell);

  // Case  (Not on nodes by construction - after second refinement)
  bdm_position[0] = 0.43291034;
  bdm_position[1] = 0.54829203;
  bdm_position[2] = 0.92717444;
  TestGridValues(scalar_field, bdm_position, cell, 0.001);

  delete cell;
  delete scalar_field->GetMesh();
  delete scalar_field;
}

// Here, we test if we can read of the solution of the grid at specific
// postions by providing the agent itself. The main reason for this test is to
// see if our accelerated search (first previous element, then neighbors, then
// checking all elements) accelerates the application as expected.
TEST(TimeDependentScalarField3dTest, StepByStepLocalization) {
  auto* scalar_field = InitializeScalarField(1);

  // Dummy simulation
  int num_agents = 10;
  Simulation simulation(
      "TimeDependentScalarField3dTest-GetSolutionAtAgentPosition");
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
  auto begin_linear_search = GetChronoTime();
  rm->ForEachAgent([&](Agent* agent) {
    double grid_value = scalar_field->GetSolutionAtAgentPosition(agent);
    EXPECT_LT(abs(agent->GetPosition().Norm() - grid_value), abs_error);
  });
  auto end_linear_search = GetChronoTime();
  rm->ForEachAgent([&](Agent* agent) {
    EXPECT_NE(agent->GetFiniteElementID(), std::numeric_limits<int>::max());
  });

  // Case 2 (Repeat exercise, this time agents should already know their
  // position and the search should be significantly faster.)
  mfem::IntegrationPoint dummy;
  rm->ForEachAgent([&](Agent* agent) {
    EXPECT_EQ(true,
              scalar_field->ContainedInElement(
                  agent->GetPosition(), agent->GetFiniteElementID(), dummy));
  });
  auto begin_no_search = GetChronoTime();
  rm->ForEachAgent([&](Agent* agent) {
    double grid_value = scalar_field->GetSolutionAtAgentPosition(agent);
    EXPECT_LT(abs(agent->GetPosition().Norm() - grid_value), abs_error);
  });
  auto end_no_search = GetChronoTime();

  // Case 3 (Repeat exercise, this time agents should already know their
  // position but we move it into a neighbor element. The search should be
  // slightly slower than in case 2 but still significantly faster than in
  // case 1)
  rm->ForEachAgent([&](Agent* agent) {
    agent->SetPosition(agent->GetPosition() + random->UniformArray<3>(0.01));
  });
  scalar_field->UpdateElementToVertexTable();
  auto begin_neighbor_search = GetChronoTime();
  rm->ForEachAgent([&](Agent* agent) {
    double grid_value = scalar_field->GetSolutionAtAgentPosition(agent);
    EXPECT_LT(abs(agent->GetPosition().Norm() - grid_value), abs_error);
  });
  auto end_neighbor_search = GetChronoTime();

  // Compute search times
  auto duration_linear_search =
      ComputeDuration(end_linear_search, begin_linear_search);
  auto duration_no_search = ComputeDuration(end_no_search, begin_no_search);
  auto duration_neighbor_search =
      ComputeDuration(end_neighbor_search, begin_neighbor_search);

  // Test if our search acceleration matches our expectation.
  EXPECT_LT(duration_no_search, duration_neighbor_search);
  EXPECT_LT(duration_no_search, duration_linear_search);
  EXPECT_LT(duration_neighbor_search, duration_linear_search);

  delete scalar_field->GetMesh();
  delete scalar_field;
}

// This test tests if we correctly compute the AgentProbabilityDistribution
// Function (1 if in agent, 0 else).
TEST(TimeDependentScalarField3dTest, AgentProbabilityDensity) {
  // Dummy simulation; todo(tobias) switch back to uniform gird after fix.
  auto set_param = [](auto* param) {
    param->bound_space = Param::BoundSpaceMode::kClosed;
    param->min_bound = 0;
    param->max_bound = 9;
    param->environment = "octree";
    param->unschedule_default_operations = {"load balancing"};
  };
  Simulation simulation(
      "TimeDependentScalarField3dTest-AgentProbabilityDensity", set_param);
  auto* rm = simulation.GetResourceManager();
  auto* scheduler = simulation.GetScheduler();
  auto* cell1 = new Cell(2.0);
  auto* cell2 = new Cell(4.0);
  auto* cell3 = new Cell(2.0);
  cell1->SetPosition({0.0, 0.0, 0.});
  cell2->SetPosition({5, 0, 0});
  cell3->SetPosition({0, 1.5, 0});
  double norm =
      1 / (cell1->GetVolume() + cell2->GetVolume() + cell3->GetVolume());
  rm->AddAgent(cell1);
  rm->AddAgent(cell2);
  rm->AddAgent(cell3);

  // Boilerplate definitions
  auto InitializeGridValues = [&](const mfem::Vector& x) { return x.Norml2(); };
  std::vector<double> parameters{0.1};
  std::vector<std::function<double(const mfem::Vector&)>> operator_functions;

  // Define a MethodOfLine Solver
  ModelInitializer::DefineMFEMSubstanceAndMesh(
      10, 10, 10, 10., 10., 10., mfem::Element::TETRAHEDRON, kSubstance1,
      "kSubstance1", 1, 3, MFEMODESolver::kBackwardEulerSolver,
      PDEOperator::kDiffusion, InitializeGridValues, parameters,
      operator_functions);

  // Get MethodOfLine scalar_field
  auto ops = scheduler->GetOps("mechanical forces");
  scheduler->UnscheduleOp(ops[0]);
  scheduler->FinalizeInitialization();
  scheduler->Simulate(1);
  auto* op = rm->GetMFEMGrid(kSubstance1).second->GetMolOperator();
  auto* pdf_functor = op->GetAgentPDFFunctor();

  // Test if there are three agents in simulation
  EXPECT_EQ(3, rm->GetNumAgents());

  // Test pdf_functor norm
  EXPECT_NE(norm, pdf_functor->GetNorm());
  op->UpdatePDFNorm();
  EXPECT_DOUBLE_EQ(norm, pdf_functor->GetNorm());

  // // Define dummy integration points to evaluate the density function
  mfem::Vector ip1 = ConvertToMFEMVector({0.1, 0.0, 0.0});  // In Cell 1
  mfem::Vector ip2 = ConvertToMFEMVector({5.5, 0.0, 0.0});  // In Cell 2
  mfem::Vector ip3 = ConvertToMFEMVector({0.0, 2.0, 0.0});  // In Cell 3
  mfem::Vector ip4 = ConvertToMFEMVector({0.0, 0.8, 0.0});  // In Cell 1 & 3
  mfem::Vector ip5 = ConvertToMFEMVector({8.0, 0.0, 0.0});  // In no cell

  // Test if values are correct
  EXPECT_DOUBLE_EQ(norm, op->EvaluateAgentPDF(ip1));
  EXPECT_DOUBLE_EQ(norm, op->EvaluateAgentPDF(ip2));
  EXPECT_DOUBLE_EQ(norm, op->EvaluateAgentPDF(ip3));
  EXPECT_DOUBLE_EQ(2 * norm, op->EvaluateAgentPDF(ip4));
  EXPECT_DOUBLE_EQ(0.0, op->EvaluateAgentPDF(ip5));
}

// Test the integration of MFEM into BioDynaMo via the Model Initializer and the
// Resource Manager.
TEST(MFEMIntegration, ModelInitializerAndRessourceManagerTest) {
  auto set_param = [](auto* param) {
    param->bound_space = Param::BoundSpaceMode::kClosed;
    param->min_bound = 1;
    param->max_bound = 9;
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
  *mesh = mfem::Mesh::MakeCartesian3D(
      10, 10, 10, mfem::Element::Type::TETRAHEDRON, 15, 15, 15);
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
      10, 10, 10, 10.0, 10.0, 10.0, mfem::Element::TETRAHEDRON, kSubstance2,
      "kSubstance2", 1, 3, MFEMODESolver::kBackwardEulerSolver,
      PDEOperator::kDiffusion, InitializeGridValues, parameters,
      operator_functions);

  // Define the third substances in our simulation
  ModelInitializer::DefineMFEMSubstanceAndMesh(
      15, 15, 15, 13.0, 13.0, 13.0, mfem::Element::TETRAHEDRON, kSubstance3,
      "kSubstance3", 2, 3, MFEMODESolver::kBackwardEulerSolver,
      PDEOperator::kDiffusion, InitializeGridValues, parameters,
      operator_functions);

  simulation.GetEnvironment()->Update();

  // Test if we have 3 registered MFEM Meshes
  EXPECT_EQ(rm->GetNumMFEMMeshes(), 3);

  // Get registered meshes and scalar_fields
  auto* mesh1 = rm->GetMFEMGrid(0).first;
  auto* mesh2 = rm->GetMFEMGrid(1).first;
  auto* mesh3 = rm->GetMFEMGrid(2).first;
  auto* scalar_field1 = rm->GetMFEMGrid(0).second;
  auto* scalar_field2 = rm->GetMFEMGrid(1).second;
  auto* scalar_field3 = rm->GetMFEMGrid(2).second;

  // Test if all pointers (mesh and scalar_field) are not nullptr and we have a
  // unique address for each of them.
  EXPECT_NE(nullptr, mesh1);
  EXPECT_NE(nullptr, mesh2);
  EXPECT_NE(nullptr, mesh3);
  EXPECT_NE(nullptr, scalar_field1);
  EXPECT_NE(nullptr, scalar_field2);
  EXPECT_NE(nullptr, scalar_field3);
  EXPECT_NE(mesh1, mesh2);
  EXPECT_NE(mesh1, mesh3);
  EXPECT_NE(mesh2, mesh3);
  EXPECT_NE(scalar_field1, scalar_field2);
  EXPECT_NE(scalar_field1, scalar_field3);
  EXPECT_NE(scalar_field2, scalar_field3);

  // Get the same pointers but from string search
  auto* mesh_1 = rm->GetMFEMGrid("kSubstance1").first;
  auto* mesh_2 = rm->GetMFEMGrid("kSubstance2").first;
  auto* mesh_3 = rm->GetMFEMGrid("kSubstance3").first;
  auto* scalar_field_1 = rm->GetMFEMGrid("kSubstance1").second;
  auto* scalar_field_2 = rm->GetMFEMGrid("kSubstance2").second;
  auto* scalar_field_3 = rm->GetMFEMGrid("kSubstance3").second;

  // Test if string and id search result in the same references
  EXPECT_EQ(mesh1, mesh_1);
  EXPECT_EQ(mesh2, mesh_2);
  EXPECT_EQ(mesh3, mesh_3);
  EXPECT_EQ(scalar_field1, scalar_field_1);
  EXPECT_EQ(scalar_field2, scalar_field_2);
  EXPECT_EQ(scalar_field3, scalar_field_3);

  // Test scheduled default operation. Implicit test ForEachMFEMGrid.
  auto* scheduler = simulation.GetScheduler();
  scheduler->Simulate(2);
  EXPECT_EQ(0.01, scalar_field1->GetSimTime());
  scheduler->Simulate(1);
  EXPECT_EQ(0.02, scalar_field2->GetSimTime());
  scheduler->Simulate(1);
  EXPECT_EQ(0.03, scalar_field3->GetSimTime());

  // Remove grids
  rm->RemoveMFEMMesh(0);
  EXPECT_EQ(rm->GetNumMFEMMeshes(), 2);
  rm->RemoveMFEMMesh(1);
  EXPECT_EQ(rm->GetNumMFEMMeshes(), 1);
  rm->RemoveMFEMMesh(2);
  EXPECT_EQ(rm->GetNumMFEMMeshes(), 0);
}

// This test is supposed to not throw an exception.
TEST(MFEMIntegration, VerifyCompatibilityBoundAndCorners) {
  auto set_param = [](auto* param) {
    param->bound_space = Param::BoundSpaceMode::kClosed;
    param->min_bound = 0.1;
    param->max_bound = 0.9;
  };
  InitializeSimulation(TEST_NAME, set_param);
}

// Here we initialize an open boundary that we do not accept for modelling at
// at the moment. Hence, we expect a fatal.
TEST(MFEMIntegrationDeathTest, VerifyCompatibilityBound) {
  EXPECT_DEATH_IF_SUPPORTED(
      {
        auto set_param = [](auto* param) {
          param->bound_space = Param::BoundSpaceMode::kOpen;
        };
        InitializeSimulation(TEST_NAME, set_param);
      },
      ".*Please make sure to use Param::BoundSpaceMode::kClosed*");
}

// Here we initialize boundaries that are not contained in the continuum. We do
// not accept such agent boundaries for modelling. Hence, we expect a fatal.
TEST(MFEMIntegrationDeathTest, VerifyCornersInMesh) {
  EXPECT_DEATH_IF_SUPPORTED(
      {
        auto set_param = [](auto* param) {
          param->bound_space = Param::BoundSpaceMode::kClosed;
          param->min_bound = -1;
          param->max_bound = 100;
        };
        InitializeSimulation(TEST_NAME, set_param);
      },
      ".*Your agent simulation is not fully contained in the FE mesh*");
}

}  // namespace experimental
}  // namespace bdm

#endif  // USE_MFEM
