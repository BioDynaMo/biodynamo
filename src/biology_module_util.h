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

#ifndef BIOLOGY_MODULE_UTIL_H_
#define BIOLOGY_MODULE_UTIL_H_

#include "event/event.h"
#include "type_util.h"
#include "variant.h"

namespace bdm {

/// BaseBiologyModule encapsulates logic to decide for which EventIds
/// a biology module should be copied or removed
struct BaseBiologyModule {
  /// Default ctor sets `copy_mask_` and remove_mask_` to 0; meaning that
  /// `Copy` and `Remove` will always return false
  BaseBiologyModule() : copy_mask_(0), remove_mask_(0) {}

  template <typename TEvent, typename TBm>
  BaseBiologyModule(const TEvent& event, TBm* other, uint64_t new_oid = 0) {
    copy_mask_ = other->copy_mask_;
    remove_mask_ = other->remove_mask_;
  }

  explicit BaseBiologyModule(EventId copy_event, EventId remove_event = 0)
      : copy_mask_(copy_event), remove_mask_(remove_event) {}

  BaseBiologyModule(std::initializer_list<EventId> copy_events,
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

  BaseBiologyModule(const BaseBiologyModule& other)
      : copy_mask_(other.copy_mask_), remove_mask_(other.remove_mask_) {}

  template <typename TEvent, typename... TBms>
  void EventHandler(const TEvent&, TBms*...) {}

  /// Function returns whether the biology module should be copied for the
  /// given event.
  bool Copy(EventId event) const { return (event & copy_mask_) != 0; }

  /// Function returns whether the biology module should be removed for the
  /// given event.
  bool Remove(EventId event) const { return (event & remove_mask_) != 0; }

 private:
  EventId copy_mask_;
  EventId remove_mask_;
  BDM_CLASS_DEF_NV(BaseBiologyModule, 2);
};

/// \brief Used for simulation objects where biology modules are not used.
/// Variant implementation does not allow `Variant<>`
/// -> `Variant<NullBiologyModule>`
struct NullBiologyModule : public BaseBiologyModule {
  NullBiologyModule() {}

  // Ctor for any event
  template <typename TEvent, typename TBm>
  NullBiologyModule(const TEvent& event, TBm* other, uint64_t new_oid = 0) {}

  // empty event handler (exising biology module won't be modified on any event)
  template <typename TEvent, typename... TBms>
  void EventHandler(const TEvent&, TBms*...) {}

  template <typename T>
  void Run(T* t) {}

  BDM_CLASS_DEF_NV(NullBiologyModule, 1);
};

/// \brief Visitor to execute the `Run` method of a biology module
/// @tparam TSimulationObject type of simulation object that owns the biology
///         module
template <typename TSimulationObject>
struct RunVisitor {
  /// @param so pointer to the simulation object on which the biology module
  ///        should be executed
  explicit RunVisitor(TSimulationObject* const so) : kSimulationObject(so) {}

  template <typename T>
  void operator()(T& t) const {
    t.Run(kSimulationObject);
  }

 private:
  TSimulationObject* const kSimulationObject;
};

}  // namespace bdm

#endif  // BIOLOGY_MODULE_UTIL_H_
