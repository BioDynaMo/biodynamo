// -----------------------------------------------------------------------------
//
// Copyright (C) The BioDynaMo Project.
// All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#include "unit/core/sim_object/sim_object_test.h"
#include "unit/test_util/test_util.h"
#include "unit/test_util/test_sim_object.h"

namespace bdm {
namespace sim_object_test_internal {

TEST(SimObjectTest, BiologyModule) {
  Simulation simulation(TEST_NAME);

  TestSimObject cell;
  double diameter = cell.GetDiameter();
  auto position = cell.GetPosition();

  cell.AddBiologyModule(new MovementModule({1, 2, 3}));
  cell.AddBiologyModule(new GrowthModule());

  cell.RunBiologyModules();

  EXPECT_NEAR(diameter + 0.5, cell.GetDiameter(), abs_error<double>::value);
  EXPECT_NEAR(position[0] + 1, cell.GetPosition()[0], abs_error<double>::value);
  EXPECT_NEAR(position[1] + 2, cell.GetPosition()[1], abs_error<double>::value);
  EXPECT_NEAR(position[2] + 3, cell.GetPosition()[2], abs_error<double>::value);
}

TEST(SimObjectTest, GetBiologyModulesTest) {
  Simulation simulation(TEST_NAME);

  // create cell and add biology modules
  TestSimObject cell;
  cell.AddBiologyModule(new GrowthModule());
  cell.AddBiologyModule(new GrowthModule());
  cell.AddBiologyModule(new MovementModule({1, 2, 3}));

  uint64_t growth_module_cnt = 0;
  uint64_t movement_module_cnt = 0;
  for(auto* bm : cell.GetAllBiologyModules()) {
    if (dynamic_cast<GrowthModule*>(bm)) {
      growth_module_cnt++;
    } else if (MovementModule* mm = dynamic_cast<MovementModule*>(bm)) {
      movement_module_cnt++;
      EXPECT_ARR_NEAR(mm->velocity_, {1, 2, 3});
    }
  }

  EXPECT_EQ(2u, growth_module_cnt);
  EXPECT_EQ(1u, movement_module_cnt);
}

TEST(SimObjectTest, BiologyModuleEventHandler) {
  Simulation simulation(TEST_NAME);

  TestSimObject cell;

  cell.AddBiologyModule(new MovementModule({1, 2, 3}));
  cell.AddBiologyModule(new GrowthModule());

  CellDivisionEvent event(1, 2, 3);
  TestSimObject copy;
  copy.EventConstructor(event, &cell, 0);
  cell.EventHandler(event, &copy);

  const auto& bms = cell.GetAllBiologyModules();
  ASSERT_EQ(1u, bms.size());
  EXPECT_TRUE(dynamic_cast<GrowthModule*>(bms[0]) != nullptr);

  const auto& copy_bms = copy.GetAllBiologyModules();
  ASSERT_EQ(1u, copy_bms.size());
  EXPECT_TRUE(dynamic_cast<GrowthModule*>(copy_bms[0]) != nullptr);
}

TEST(SimObjectTest, RemoveBiologyModule) {
  Simulation simulation(TEST_NAME);

  TestSimObject cell;

  // add RemoveModule as first one! If removal while iterating over it is not
  // implemented correctly, MovementModule will not be executed.
  cell.AddBiologyModule(new RemoveModule());
  cell.AddBiologyModule(new MovementModule({1, 2, 3}));
  cell.AddBiologyModule(new GrowthModule());

  // RemoveModule should remove itself
  cell.RunBiologyModules();

  const auto& bms = cell.GetAllBiologyModules();
  ASSERT_EQ(2u, bms.size());
  EXPECT_TRUE(dynamic_cast<MovementModule*>(bms[0]) != nullptr);
  EXPECT_TRUE(dynamic_cast<GrowthModule*>(bms[1]) != nullptr);
  // check if MovementModule and GrowthModule have been executed correctly.
  EXPECT_ARR_NEAR({1, 2, 3}, cell.GetPosition());
  EXPECT_NEAR(0.5, cell.GetDiameter(), abs_error<double>::value);

  cell.AddBiologyModule(new RemoveModule());
  ASSERT_EQ(3u, bms.size());
  auto* to_be_removed = dynamic_cast<RemoveModule*>(bms[2]);
  cell.RemoveBiologyModule(to_be_removed);
  ASSERT_EQ(2u, bms.size());
}

// FIXME
// TEST(SimObjectUtilTest, ForEachDataMember) {
//   std::vector<Neurite> neurites;
//   Neuron neuron1(neurites, std::array<double, 3>{4, 5, 6});
//
//   auto neurons = Neuron::NewEmptySoa();
//   neurons.push_back(neuron1);
//
//   uint16_t counter = 0;
//   auto verify = [&](auto* data_member, const std::string& dm_name) {
//     ASSERT_EQ(1u, data_member->size());
//     counter++;
//     if (dm_name != "neurites_" && dm_name != "position_" &&
//         dm_name != "diameter_" && dm_name != "biology_modules_" &&
//         dm_name != "uid_" && dm_name != "box_idx_" &&
//         dm_name != "run_bm_loop_idx_" && dm_name != "numa_node_") {
//       FAIL() << "Data member " << dm_name << "does not exist" << std::endl;
//     }
//   };
//
//   neurons.ForEachDataMember(verify);
//   EXPECT_EQ(8u, counter);
// }
//
// TEST(SimObjectUtilTest, ForEachDataMemberIn) {
//   std::vector<Neurite> neurites;
//   Neuron neuron1(neurites, std::array<double, 3>{4, 5, 6});
//
//   auto neurons = Neuron::NewEmptySoa();
//   neurons.push_back(neuron1);
//
//   uint16_t counter = 0;
//   auto verify = [&](auto* data_member, const std::string& dm_name) {
//     ASSERT_EQ(1u, data_member->size());
//     counter++;
//     if (dm_name != "neurites_" && dm_name != "position_") {
//       FAIL() << "Lambda must not be called for data member " << dm_name
//              << std::endl;
//     }
//   };
//
//   neurons.ForEachDataMemberIn(std::set<std::string>{"neurites_", "position_"},
//                               verify);
//   EXPECT_EQ(2u, counter);
// }
//
// struct VerifyPosition {
//   void operator()(std::vector<std::array<double, 3>>* data_member,
//                   const std::string& dm_name) {
//     ASSERT_EQ(1u, data_member->size());
//     if (dm_name == "position_") {
//       EXPECT_EQ(4, (*data_member)[0][0]);
//       EXPECT_EQ(5, (*data_member)[0][1]);
//       EXPECT_EQ(6, (*data_member)[0][2]);
//     } else {
//       FAIL() << "Functor must not be called for data member " << dm_name
//              << std::endl;
//     }
//   }
//
//   void operator()(...) { FAIL() << "Should not be called" << std::endl; }
// };
//
// // for one data member check if the pointer contains the right data
// TEST(SimObjectUtilTest, ForEachDataMemberInDetailed) {
//   std::vector<Neurite> neurites;
//   Neuron neuron1(neurites, std::array<double, 3>{4, 5, 6});
//
//   auto neurons = Neuron::NewEmptySoa();
//   neurons.push_back(neuron1);
//
//   neurons.ForEachDataMemberIn(std::set<std::string>{"position_"},
//                               VerifyPosition());
// }
//
// TEST(SimObjectUtilTestDeathTest, ForEachDataMemberInNonExistantDm) {
//   auto neurons = Neuron::NewEmptySoa();
//   auto lambda = [&]() {
//     std::set<std::string> dm = {"position_", "does_not_exist"};
//     neurons.ForEachDataMemberIn(dm, [](auto*, const std::string&) {});
//   };
//
//   ASSERT_DEATH(lambda(),
//                ".*Please check your config file. The following data members do "
//                "not exist: does_not_exist, .*");
// }
//
// TEST(SimObjectTest, GetSoPtr) {
//   Simulation simulation(TEST_NAME);
//   auto* rm = simulation.GetResourceManager();
//
//   for (uint64_t i = 0; i < 10; i++) {
//     rm->push_back(new TestSimObject());
//   }
//   EXPECT_EQ(10u, rm->GetNumSimObjects());
//
//   for (uint64_t i = 0; i < 10; i++) {
//     SoPointer<SoaNeuron, Soa> expected((*neurons)[i].GetUid());
//     EXPECT_EQ(expected, (*neurons)[i].GetSoPtr());
//   }
// }

}  // namespace sim_object_test_internal
}  // namespace bdm
