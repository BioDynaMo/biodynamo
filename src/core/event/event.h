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

#ifndef CORE_EVENT_EVENT_H_
#define CORE_EVENT_EVENT_H_

#include <limits>
#include <mutex>
#include "core/util/log.h"

namespace bdm {

/// EventId is used inside biology modules to determine if a biology module
/// should be copied if a new simulation object is created.
/// Possible events are cell division, neurite branching, ...\n
/// EventId invariant: the number of bits set to 1 must be 1.
using EventId = uint64_t;

/// Biology module event representing the union of all events.\n
/// Used to create a biology module  which is copied for every event.
/// @see `BaseBiologyModule`
const EventId gAllEventIds = std::numeric_limits<uint64_t>::max();

/// Biology module event representing the null element = empty set of events.
/// @see `BaseBiologyModule`
const EventId gNullEventId = 0;

/// This class generates unique ids for biology module events satisfying the
/// EventId invariant. Thread safe.
class UniqueEventIdFactory {
 public:
  UniqueEventIdFactory(const UniqueEventIdFactory&) = delete;

  static UniqueEventIdFactory* Get() {
    static UniqueEventIdFactory kInstance;
    return &kInstance;
  }

  EventId NewUniqueEventId() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    constexpr uint64_t kOne = 1;
    if (counter_ == 64) {
      Log::Fatal("UniqueEventIdFactory",
                 "BioDynaMo only supports 64 unique EventIds."
                 " You requested a 65th one.");
    }
    return kOne << counter_++;
  }

 private:
  UniqueEventIdFactory() {}
  std::recursive_mutex mutex_;
  uint64_t counter_ = 0;
};

struct Event {
  virtual ~Event() {}

  virtual EventId GetId() const = 0;
};

}  // namespace bdm

#endif  // CORE_EVENT_EVENT_H_
