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

/// BaseBehavior encapsulates logic to decide for which EventIds
/// a behavior should be copied or removed
struct BaseBehavior {
  /// Default ctor sets `copy_mask_` and remove_mask_` to 0; meaning that
  /// `Copy` and `Remove` will always return false
  BaseBehavior() : copy_mask_(0), remove_mask_(0) {}

  explicit BaseBehavior(EventId copy_event, EventId remove_event = 0)
      : copy_mask_(copy_event), remove_mask_(remove_event) {}

  BaseBehavior(std::initializer_list<EventId> copy_events,
                    std::initializer_list<EventId> remove_events = {}) {
    // copy mask
    copy_mask_ = 0;
    for (EventId event : copy_events) {
      copy_mask_ |= event;
    }
    // delete mask
    remove_mask_ = 0;
    for (EventId event : remove_events) {
      remove_mask_ |= event;
    }
  }

  BaseBehavior(const Event& event, BaseBehavior* other,
                    uint64_t new_oid = 0) {
    copy_mask_ = other->copy_mask_;
    remove_mask_ = other->remove_mask_;
  }

  BaseBehavior(const BaseBehavior& other)
      : copy_mask_(other.copy_mask_), remove_mask_(other.remove_mask_) {}

  virtual ~BaseBehavior() {}

  /// Create a new instance of this object using the default constructor.
  virtual BaseBehavior* GetInstance(const Event& event,
                                         BaseBehavior* other,
                                         uint64_t new_oid = 0) const = 0;

  /// Create a copy of this behavior.
  virtual BaseBehavior* GetCopy() const = 0;

  virtual void EventHandler(const Event& event, BaseBehavior* other1,
                            BaseBehavior* other2 = nullptr) {}

  virtual void Run(Agent* agent) = 0;

  /// Function returns whether the behavior should be copied for the
  /// given event.
  bool Copy(EventId event) const { return (event & copy_mask_) != 0; }

  /// Function returns whether the behavior should be removed for the
  /// given event.
  bool Remove(EventId event) const { return (event & remove_mask_) != 0; }

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
  EventId copy_mask_;
  EventId remove_mask_;
  BDM_CLASS_DEF(BaseBehavior, 2);
};

/// Inserts boilerplate code for stateless behaviors
#define BDM_STATELESS_BEHAVIOR_HEADER(class_name, base_class, class_version_id)      \
 public:                                                                       \
  /** Empty default event constructor, because module does not have state. */  \
  class_name(const Event& event, BaseBehavior* other,                     \
             uint64_t new_oid = 0)                                             \
      : base_class(event, other, new_oid) {}                                   \
                                                                               \
  /** Event handler not needed, because this module does not have state. */    \
                                                                               \
  /** Create a new instance of this object using the default constructor. */   \
  BaseBehavior* GetInstance(const Event& event, BaseBehavior* other, \
                                 uint64_t new_oid = 0) const override {        \
    return new class_name(event, other, new_oid);                              \
  }                                                                            \
                                                                               \
  /** Create a copy of this behavior. */                                 \
  BaseBehavior* GetCopy() const override {                                \
    return new class_name(*this);                                              \
  }                                                                            \
                                                                               \
 private:                                                                      \
  BDM_CLASS_DEF_OVERRIDE(class_name, class_version_id);                        \
                                                                               \
 public:

/// Inserts boilerplate code for behaviors with state
#define BDM_BEHAVIOR_HEADER(class_name, base_class, class_version_id)                \
 public:                                                                       \
  /** Create a new instance of this object using the default constructor. */   \
  BaseBehavior* GetInstance(const Event& event, BaseBehavior* other, \
                                 uint64_t new_oid = 0) const override {        \
    return new class_name(event, other, new_oid);                              \
  }                                                                            \
                                                                               \
  /** Create a copy of this behavior. */                                 \
  BaseBehavior* GetCopy() const override {                                \
    return new class_name(*this);                                              \
  }                                                                            \
                                                                               \
 private:                                                                      \
  BDM_CLASS_DEF_OVERRIDE(class_name, class_version_id);                        \
                                                                               \
 public:

}  // namespace bdm

#endif  // CORE_BEHAVIOR_BEHAVIOR_H_