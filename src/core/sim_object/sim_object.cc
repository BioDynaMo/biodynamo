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
#include "core/resource_manager.h"
#include "core/sim_object/so_pointer.h"
#include "core/sim_object/so_uid.h"
#include "core/simulation.h"
#include "core/util/log.h"
#include "core/util/macros.h"
#include "core/util/root.h"
#include "core/util/type.h"
#include "core/execution_context/in_place_exec_ctxt.h"

namespace bdm {

const std::string SimObject::GetScalarTypeName() { return "Cell"; }

SimObject::SimObject() { uid_ = SoUidGenerator::Get()->NewSoUid(); }

SimObject::~SimObject() {
  for(auto* el : biology_modules_) {
    delete el;
  }
}

SimObject::SimObject(TRootIOCtor *io_ctor) {}

SimObject::SimObject(const SimObject &other) = default;

void SimObject::RunDiscretization() {}

SoUid SimObject::GetUid() const { return uid_; }

uint32_t SimObject::GetBoxIdx() const { return box_idx_; }

void SimObject::SetBoxIdx(uint32_t idx) { box_idx_ = idx; }

SoPointer SimObject::GetSoPtr() const { return SoPointer(uid_); }

// ---------------------------------------------------------------------------
// Biology modules

void SimObject::AddBiologyModule(BaseBiologyModule* module) {
  biology_modules_.emplace_back(module);
}

void SimObject::RemoveBiologyModule(const BaseBiologyModule *remove_module) {
  for (unsigned int i = 0; i < biology_modules_.size(); i++) {
    if (biology_modules_[i] == remove_module) {
      biology_modules_.erase(biology_modules_.begin() + i);
      // if remove_module was before or at the current run_bm_loop_idx_,
      // correct it by subtracting one.
      run_bm_loop_idx_ -= i > run_bm_loop_idx_ ? 0 : 1;
    }
  }
}

void SimObject::RunBiologyModules() {
  for (auto* bm : biology_modules_) {
    bm->Run(this);
  }
}

const auto &SimObject::GetAllBiologyModules() const { return biology_modules_; }
// ---------------------------------------------------------------------------

void SimObject::RemoveFromSimulation() const {
  Simulation::GetActive()->GetExecutionContext()->RemoveFromSimulation(uid_);
}

void SimObject::EventConstructor(const Event& event, SimObject* other, uint64_t new_oid) {
  box_idx_ = other->GetBoxIdx();
  // biology modules
  auto* other_bms = &(other->biology_modules_);
  // copy biology_modules_ to me
  CopyBiologyModules(event, other_bms);
}

void SimObject::EventHandler(const Event &event, SimObject *other1, SimObject* other2) {
  // call event handler for biology modules
  auto *left_bms = &(other1->biology_modules_);
  auto *right_bms = &(other2->biology_modules_);
  BiologyModuleEventHandler(event, left_bms, right_bms);
}

void SimObject::CopyBiologyModules(const Event& event, decltype(biology_modules_) *dest) {
  for (auto* bm : biology_modules_) {
    if(bm->Copy(event.GetId())) {
      auto* new_bm = bm->GetInstance();
      new_bm->EventConstructor(event, bm);
      dest->push_back(new_bm);
    }
  }
}

void SimObject::BiologyModuleEventHandler(const Event &event, decltype(biology_modules_) *other1,
                               decltype(biology_modules_) *other2) {
  // call event handler for biology modules
  uint64_t cnt = 0;
  for(auto* bm : biology_modules_) {
    bool copy = bm->Copy(event.GetId());
    if (!bm->Remove(event.GetId())) {
      if(copy) {
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
