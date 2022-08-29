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

#include "unit/core/environment/custom_environment_test.h"
#include "core/agent/cell.h"
#include "core/environment/environment.h"
#include "gtest/gtest.h"
#include "unit/test_util/test_util.h"

#include <string>

namespace bdm {

class MyEnvironment : public Environment {
 public:
  struct Criteria {
    explicit Criteria(std::string c) : city(c) {}
    std::string city;
  };

  void Clear() override {}

  std::array<int32_t, 6> GetDimensions() const override {
    return {0, 0, 0, 0, 0, 0};
  }

  std::array<int32_t, 2> GetDimensionThresholds() const override {
    return {0, 0};
  }

  LoadBalanceInfo* GetLoadBalanceInfo() override {
    Log::Fatal(
        "MyEnvironment::GetLoadBalanceInfo",
        "You tried to call GetLoadBalanceInfo in an environment that does "
        "not support it.");
    return nullptr;
  }

  Environment::NeighborMutexBuilder* GetNeighborMutexBuilder() override {
    return nullptr;
  };

  std::unordered_map<std::string, std::vector<Agent*>> agents_per_city_;

  // In this environment a neighboring agent is an agent who is from the same
  // `city` as the query agent
  void ForEachNeighbor(Functor<void, Agent*>& lambda, const Agent& query,
                       void* criteria) override {
    // Even though the criteria could have been typed as a std::string, this
    // example shows that you can wrap any number of criteria in a struct
    auto casted_criteria = static_cast<Criteria*>(criteria);
    for (auto neighbor : agents_per_city_[casted_criteria->city]) {
      if (neighbor != &query) {
        lambda(neighbor);
      }
    }
  }
  void ForEachNeighbor(Functor<void, Agent*, real_t>& lambda,
                       const Agent& query, real_t squared_radius) override{};

  void ForEachNeighbor(Functor<void, Agent*, real_t>& lambda,
                       const Real3& query_position, real_t squared_radius,
                       const Agent* query_agent = nullptr) override{};

  int GetNumUpdates() { return num_updates_; }

 protected:
  void UpdateImplementation() override { num_updates_ += 1; }
  int num_updates_ = 0;
};

struct FindNeighborsInCity : public Functor<void, Agent*> {
  std::unordered_map<AgentUid, std::vector<AgentUid>>* neighbors_;
  APerson* query_;
  FindNeighborsInCity(
      std::unordered_map<AgentUid, std::vector<AgentUid>>* neighbors,
      APerson* person)
      : neighbors_(neighbors), query_(person) {}

  void operator()(Agent* neighbor) override {
    auto nuid = neighbor->GetUid();
    auto uid = query_->GetUid();
    auto neighbor_person = dynamic_cast<APerson*>(neighbor);
    if (neighbor_person->age_ >= query_->age_ &&
        neighbor_person->gender_ != query_->gender_) {
      (*neighbors_)[uid].push_back(nuid);
    }
  }
};

TEST(CustomEnvironmentTest, CustomCriteria) {
  Simulation simulation(TEST_NAME);

  // Populate the environment with agents (would normally be done in the
  // Environment::Update() call)
  MyEnvironment env;
  auto* person0 = new APerson(27, 1, "Geneva");
  auto* person1 = new APerson(42, 1, "Geneva");
  auto* person2 = new APerson(43, 0, "Geneva");
  auto* person3 = new APerson(21, 0, "Amsterdam");
  auto* person4 = new APerson(23, 0, "Amsterdam");
  auto* person5 = new APerson(25, 1, "Amsterdam");
  env.agents_per_city_["Geneva"].push_back(person0);
  env.agents_per_city_["Geneva"].push_back(person1);
  env.agents_per_city_["Geneva"].push_back(person2);
  env.agents_per_city_["Amsterdam"].push_back(person3);
  env.agents_per_city_["Amsterdam"].push_back(person4);
  env.agents_per_city_["Amsterdam"].push_back(person5);

  std::unordered_map<AgentUid, std::vector<AgentUid>> neighbors;

  MyEnvironment::Criteria c("Geneva");
  FindNeighborsInCity functor(&neighbors, person0);
  env.ForEachNeighbor(functor, *person0, &c);

  FindNeighborsInCity functor1(&neighbors, person1);
  env.ForEachNeighbor(functor1, *person1, &c);

  EXPECT_EQ(neighbors[person0->GetUid()].size(), 1u);
  EXPECT_EQ(neighbors[person0->GetUid()][0], person2->GetUid());
  EXPECT_EQ(neighbors[person1->GetUid()].size(), 1u);
}

TEST(CustomEnvironmentTest, NumberOfUpdates) {
  auto set_param = [&](Param* param) {
    param->unschedule_default_operations = {"load balancing"};
  };
  Simulation sim(TEST_NAME, set_param);

  auto* rm = sim.GetResourceManager();
  auto* scheduler = sim.GetScheduler();
  MyEnvironment* env_ptr = new MyEnvironment();
  sim.SetEnvironment(env_ptr);

  EXPECT_EQ(env_ptr, sim.GetEnvironment());

  Cell* cell = new Cell(1.0);
  rm->AddAgent(cell);
  scheduler->Simulate(1);
  // Updates: Schduler::Initialize(), Tear down op
  EXPECT_EQ(2, env_ptr->GetNumUpdates());

  scheduler->Simulate(1);
  // Updates: Schduler::Initialize(), Tear down op
  EXPECT_EQ(4, env_ptr->GetNumUpdates());

  scheduler->Simulate(2);
  // Updates: Schduler::Initialize(), Tear down op, Tear down op
  EXPECT_EQ(7, env_ptr->GetNumUpdates());

  rm->AddAgent(new Cell(1.0));
  scheduler->Simulate(2);
  // Updates: Schduler::Initialize(), Tear down op, Tear down op
  EXPECT_EQ(10, env_ptr->GetNumUpdates());
}

}  // namespace bdm
