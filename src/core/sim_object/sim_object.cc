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

#include "core/sim_object/sim_object.h"

#include <algorithm>
#include <cassert>
#include <limits>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "core/biology_module/biology_module.h"
#include "core/event/event.h"
#include "core/execution_context/in_place_exec_ctxt.h"
#include "core/grid.h"
#include "core/resource_manager.h"
#include "core/sim_object/so_uid.h"
#include "core/simulation.h"
#include "core/util/log.h"
#include "core/util/macros.h"
#include "core/util/root.h"
#include "core/util/type.h"

namespace bdm {

SimObject::SimObject() { uid_ = SoUidGenerator::Get()->NewSoUid(); }

SimObject::SimObject(const Event& event, SimObject* other, uint64_t new_oid)
    : SimObject() {
  box_idx_ = other->GetBoxIdx();
  // biology modules
  auto* other_bms = &(other->biology_modules_);
  // copy biology_modules_ to me
  CopyBiologyModules(event, other_bms);
}

SimObject::SimObject(TRootIOCtor* io_ctor) {}

SimObject::SimObject(const SimObject& other)
    : uid_(other.uid_),
      box_idx_(other.box_idx_),
      run_bm_loop_idx_(other.run_bm_loop_idx_),
      run_displacement_for_all_next_ts_(
          other.run_displacement_for_all_next_ts_),
      run_displacement_next_ts_(other.run_displacement_next_ts_) {
  for (auto* module : other.biology_modules_) {
    biology_modules_.push_back(module->GetCopy());
  }
}

SimObject::~SimObject() {
  for (auto* el : biology_modules_) {
    delete el;
  }
}

void SimObject::ApplyRunDisplacementForAllNextTs() {
  if (!Simulation::GetActive()->GetParam()->detect_static_sim_objects_) {
    run_displacement_next_ts_ = true;
    return;
  }

  if (!run_displacement_for_all_next_ts_) {
    return;
  }
  run_displacement_for_all_next_ts_ = false;
  run_displacement_next_ts_ = true;
  auto* ctxt = Simulation::GetActive()->GetExecutionContext();
  ctxt->ForEachNeighbor(
      [this](const SimObject* neighbor, double squared_distance) {
        double distance = this->GetDiameter() + neighbor->GetDiameter();
        if (squared_distance < distance * distance) {
          neighbor->SetRunDisplacementNextTimestep(true);
        }
      },
      *this);
}

void SimObject::RunDiscretization() {}

void SimObject::AssignNewUid() { uid_ = SoUidGenerator::Get()->NewSoUid(); }

SoUid SimObject::GetUid() const { return uid_; }

uint32_t SimObject::GetBoxIdx() const { return box_idx_; }

void SimObject::SetBoxIdx(uint32_t idx) { box_idx_ = idx; }

// ---------------------------------------------------------------------------
// Biology modules

void SimObject::AddBiologyModule(BaseBiologyModule* module) {
  biology_modules_.emplace_back(module);
}

void SimObject::RemoveBiologyModule(const BaseBiologyModule* remove_module) {
  for (unsigned int i = 0; i < biology_modules_.size(); i++) {
    if (biology_modules_[i] == remove_module) {
      delete remove_module;
      biology_modules_.erase(biology_modules_.begin() + i);
      // if remove_module was before or at the current run_bm_loop_idx_,
      // correct it by subtracting one.
      run_bm_loop_idx_ -= i > run_bm_loop_idx_ ? 0 : 1;
    }
  }
}

void SimObject::RunBiologyModules() {
  for (run_bm_loop_idx_ = 0; run_bm_loop_idx_ < biology_modules_.size();
       ++run_bm_loop_idx_) {
    auto* module = biology_modules_[run_bm_loop_idx_];
    module->Run(this);
  }
}

const std::vector<BaseBiologyModule*>& SimObject::GetAllBiologyModules() const {
  return biology_modules_;
}
// ---------------------------------------------------------------------------

void SimObject::RemoveFromSimulation() const {
  Simulation::GetActive()->GetExecutionContext()->RemoveFromSimulation(uid_);
}

void SimObject::EventHandler(const Event& event, SimObject* other1,
                             SimObject* other2) {
  // Run displacement if a new sim object has been created with an event.
  SetRunDisplacementForAllNextTs();
  // call event handler for biology modules
  auto* left_bms = other1 == nullptr ? nullptr : &(other1->biology_modules_);
  auto* right_bms = other2 == nullptr ? nullptr : &(other2->biology_modules_);
  BiologyModuleEventHandler(event, left_bms, right_bms);
}

void SimObject::CopyBiologyModules(const Event& event,
                                   decltype(biology_modules_) * other) {
  for (auto* bm : *other) {
    if (bm->Copy(event.GetId())) {
      auto* new_bm = bm->GetInstance(event, bm);
      biology_modules_.push_back(new_bm);
    }
  }
}

void SimObject::BiologyModuleEventHandler(const Event& event,
                                          decltype(biology_modules_) * other1,
                                          decltype(biology_modules_) * other2) {
  // call event handler for biology modules
  uint64_t cnt = 0;
  for (auto* bm : biology_modules_) {
    bool copy = bm->Copy(event.GetId());
    if (!bm->Remove(event.GetId())) {
      if (copy) {
        auto* other1_bm = other1 != nullptr ? (*other1)[cnt] : nullptr;
        auto* other2_bm = other2 != nullptr ? (*other2)[cnt] : nullptr;
        bm->EventHandler(event, other1_bm, other2_bm);
      } else {
        bm->EventHandler(event, nullptr, nullptr);
      }
    }
    cnt += copy ? 1 : 0;
  }

  // remove biology modules from current
  for (auto it = biology_modules_.begin(); it != biology_modules_.end();) {
    auto* bm = *it;
    if (bm->Remove(event.GetId())) {
      delete *it;
      it = biology_modules_.erase(it);
    } else {
      ++it;
    }
  }
}

}  // namespace bdm
