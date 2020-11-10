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

#ifndef CORE_BEHAVIOR_BEHAVIOR_H_
#define CORE_BEHAVIOR_BEHAVIOR_H_

#include "core/event/event.h"
#include "core/agent/agent.h"
#include "core/util/type.h"

namespace bdm {

/// Behavior encapsulates logic to decide for which NewAgentEventUids
/// a behavior should be copied or removed
struct Behavior {
  /// Default ctor sets `copy_mask_` and remove_mask_` to 0; meaning that
  /// `Copy` and `Remove` will always return false
  Behavior() : copy_mask_(0), remove_mask_(0) {}

  virtual ~Behavior() {}

  /// Create a new instance of this object using the default constructor.
  virtual Behavior* New() const = 0;

  /// Create a new copy of this behavior.
  virtual Behavior* NewCopy() const = 0;

  virtual void Initialize(const NewAgentEvent& event) {
    copy_mask_ = event.existing_behavior->copy_mask_;
    remove_mask_ = event.existing_behavior->remove_mask_;
  }
  
  virtual void Update(const NewAgentEvent& event) {}

  virtual void Run(Agent* agent) = 0;

  /// Always copy this behavior to new agents
  void CopyToNewAlways() { copy_mask_ = std::numeric_limits<NewAgentEventUid>::max(); }
  /// Never copy this behavior to new agents
  void CopyToNewNever() { copy_mask_ = 0; }
  /// If a new agent will be created with one of 
  /// these NewAgentEventUids, this behavior will be copied to the new agent. 
  void CopyToNewIf(const std::initializer_list<NewAgentEventUid>& uids) { 
    for (auto& uid : uids) {
      copy_mask_ |= uid;
    }
  }

  /// Always remove this behavior from the existing agent if a new agent is created.
  void RemoveFromExistingAlways() { remove_mask_ = std::numeric_limits<NewAgentEventUid>::max(); }
  /// Never remove this behavior from the existing agent if a new agent is created.
  void RemoveFromExistingNever() { remove_mask_ = 0; }
  /// If a new agent will be created with one of these
  /// NewAgentEventUids, this behavior will be removed from the existing agent. 
  void RemoveFromExistingIf(const std::initializer_list<NewAgentEventUid>& uids) { 
    for (auto& uid : uids) {
      remove_mask_ |= uid;
    }
  }

  /// Function returns whether the behavior will be copied for the
  /// given event.
  bool WillBeCopied(NewAgentEventUid event) const { return (event & copy_mask_) != 0; }

  /// Function returns whether the behavior will be removed for the
  /// given event.
  bool WillBeRemoved(NewAgentEventUid event) const { return (event & remove_mask_) != 0; }

  void* operator new(size_t size) {  // NOLINT
    auto* mem_mgr = Simulation::GetActive()->GetMemoryManager();
    if (mem_mgr) {
      return mem_mgr->New(size);
    } else {
      return malloc(size);
    }
  }

  void operator delete(void* p) {  // NOLINT
    auto* mem_mgr = Simulation::GetActive()->GetMemoryManager();
    if (mem_mgr) {
      mem_mgr->Delete(p);
    } else {
      free(p);
    }
  }

 private:
  NewAgentEventUid copy_mask_ = 0;
  NewAgentEventUid remove_mask_ = 0;
  BDM_CLASS_DEF(Behavior, 2);
};

/// Inserts boilerplate code for behaviors with state
#define BDM_BEHAVIOR_HEADER(class_name, base_class, class_version_id)                \
 public:                                                                       \
  using Base = base_class; \
  /** Create a new instance of this object using the default constructor. */   \
  Behavior* New() const override { return new class_name(); }             \
  /** Create a new instance of this object using the copy constructor. */   \
  Behavior* NewCopy() const override { return new class_name(*this);    }          \
                                                                               \
 private:                                                                      \
  BDM_CLASS_DEF_OVERRIDE(class_name, class_version_id);                        \
                                                                               \
 public:

}  // namespace bdm

#endif  // CORE_BEHAVIOR_BEHAVIOR_H_
