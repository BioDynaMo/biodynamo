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
#include "core/resource_manager.h"
#include "unit/test_util/test_sim_object.h"
#include "unit/test_util/test_util.h"

namespace bdm {
namespace sim_object_test_internal {

TEST(SimObjectTest, CopyCtor) {
  TestSimObject cell;
  cell.SetBoxIdx(123);
  GrowthModule* gm = new GrowthModule();
  gm->growth_rate_ = 321;
  cell.AddBiologyModule(gm);

  TestSimObject copy(cell);
  EXPECT_EQ(123u, copy.GetBoxIdx());
  EXPECT_EQ(cell.GetUid(), copy.GetUid());
  ASSERT_EQ(1u, copy.GetAllBiologyModules().size());
  GrowthModule* copy_gm =
      dynamic_cast<GrowthModule*>(copy.GetAllBiologyModules()[0]);
  EXPECT_TRUE(gm != copy_gm);
  EXPECT_EQ(321, copy_gm->growth_rate_);
}

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
  for (auto* bm : cell.GetAllBiologyModules()) {
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
  TestSimObject copy(event, &cell, 0);
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

struct Visitor1 : public SoVisitor {
  uint16_t counter_ = 0;

  void Visit(const std::string& dm_name, size_t type_hash_code,
             void* data) override {
    counter_++;
    if (dm_name != "position_" && dm_name != "run_bm_loop_idx_" &&
        dm_name != "diameter_" && dm_name != "biology_modules_" &&
        dm_name != "uid_" && dm_name != "box_idx_") {
      FAIL() << "Data member " << dm_name << " does not exist" << std::endl;
    }
  }
};

TEST(SimObjectUtilTest, ForEachDataMember) {
  TestSimObject so;
  Visitor1 visitor;
  so.ForEachDataMember(&visitor);
  EXPECT_EQ(6u, visitor.counter_);
}

struct Visitor2 : public SoVisitor {
  uint16_t counter_ = 0;

  void Visit(const std::string& dm_name, size_t type_hash_code,
             void* data) override {
    counter_++;
    if (dm_name != "uid_" && dm_name != "position_") {
      FAIL() << "Lambda must not be called for data member " << dm_name
             << std::endl;
    }
  }
};

TEST(SimObjectUtilTest, ForEachDataMemberIn) {
  TestSimObject so;
  Visitor2 visitor;
  so.ForEachDataMemberIn(std::set<std::string>{"uid_", "position_"}, &visitor);
  EXPECT_EQ(2u, visitor.counter_);
}

struct VerifyPosition : public SoVisitor {
  void Visit(const std::string& dm_name, size_t type_hash_code,
             void* data) override {
    if (dm_name != "position_") {
      FAIL() << "Functor must not be called for data member " << dm_name
             << std::endl;
    }
    using PosType = std::array<double, 3>;
    if (type_hash_code == typeid(PosType).hash_code()) {
      auto* pos = static_cast<PosType*>(data);
      EXPECT_EQ(4, (*pos)[0]);
      EXPECT_EQ(5, (*pos)[1]);
      EXPECT_EQ(6, (*pos)[2]);
    } else {
      FAIL() << "type_hash_code did not match std::array<double, 3>"
             << std::endl;
    }
  }
};

// for one data member check if the pointer contains the right data
TEST(SimObjectUtilTest, ForEachDataMemberInDetailed) {
  TestSimObject so;
  so.SetPosition({4, 5, 6});
  VerifyPosition visitor;
  so.ForEachDataMemberIn(std::set<std::string>{"position_"}, &visitor);
}

TEST(SimObjectTest, GetSoPtr) {
  Simulation simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();

  std::vector<SimObject*> sim_objects;
  for (uint64_t i = 0; i < 10; i++) {
    auto* so = new TestSimObject();
    rm->push_back(so);
    sim_objects.push_back(so);
  }
  EXPECT_EQ(10u, rm->GetNumSimObjects());

  for (uint64_t i = 0; i < 10; i++) {
    SoPointer<TestSimObject> expected(sim_objects[i]->GetUid());
    EXPECT_EQ(expected, sim_objects[i]->GetSoPtr<TestSimObject>());

    SoPointer<SimObject> expected1(sim_objects[i]->GetUid());
    EXPECT_EQ(expected1, sim_objects[i]->GetSoPtr());
  }
}

}  // namespace sim_object_test_internal
}  // namespace bdm
