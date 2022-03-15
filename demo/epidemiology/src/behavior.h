// -----------------------------------------------------------------------------
//
// Copyright (C) Lukas Breitwieser.
// All Rights Reserved.
//
// -----------------------------------------------------------------------------

#ifndef BEHAVIOR_H_
#define BEHAVIOR_H_

#include "core/behavior/behavior.h"

#include "person.h"
#include "sim-param.h"

namespace bdm {

// -----------------------------------------------------------------------------
struct Infection : public Behavior {
  BDM_BEHAVIOR_HEADER(Infection, Behavior, 1);

  Infection() {}
  virtual ~Infection() {}

  void Run(Agent* a) override {
    auto* sim = Simulation::GetActive();
    auto* random = sim->GetRandom();
    auto* param = sim->GetParam();
    auto* sparam = param->Get<SimParam>();

    auto* person = bdm_static_cast<Person*>(a);
    if (person->state_ == kSusceptible &&
        random->Uniform(0, 1) <= sparam->infection_probablity) {
      auto* ctxt = sim->GetExecutionContext();
      auto check_surrounding =
          L2F([&](Agent* neighbor, real squared_distance) {
            auto* other = bdm_static_cast<const Person*>(neighbor);
            if (other->state_ == State::kInfected) {
              person->state_ = State::kInfected;
            }
          });
      ctxt->ForEachNeighbor(check_surrounding, *person,
                            sparam->infection_radius);
    }
  }
};

// -----------------------------------------------------------------------------
struct Recovery : public Behavior {
  BDM_BEHAVIOR_HEADER(Recovery, Behavior, 1);

  Recovery() {}
  virtual ~Recovery() {}

  void Run(Agent* a) override {
    auto* person = bdm_static_cast<Person*>(a);
    if (person->state_ == kInfected) {
      auto* sim = Simulation::GetActive();
      auto* random = sim->GetRandom();
      auto* sparam = sim->GetParam()->Get<SimParam>();
      if (random->Uniform(0, 1) <= sparam->recovery_probability) {
        person->state_ = State::kRecovered;
      }
    }
  }
};

// -----------------------------------------------------------------------------
struct RandomMovement : public Behavior {
  BDM_BEHAVIOR_HEADER(RandomMovement, Behavior, 1);

  RandomMovement() {}
  virtual ~RandomMovement() {}

  void Run(Agent* agent) override {
    auto* sim = Simulation::GetActive();
    auto* random = sim->GetRandom();
    auto* sparam = sim->GetParam()->Get<SimParam>();

    const auto& position = agent->GetPosition();
    auto rand_movement = random->UniformArray<3>(-1, 1);
    rand_movement.Normalize();
    auto new_pos = position + rand_movement * sparam->agent_speed;
    agent->SetPosition(new_pos);
  }
};

}  // namespace bdm

#endif  // BEHAVIOR_H_
