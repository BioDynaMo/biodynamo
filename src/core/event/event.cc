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

#include "core/event/event.h"

namespace bdm {

UniqueEventIdFactory* UniqueEventIdFactory::Get() {
  static UniqueEventIdFactory kInstance;
  return &kInstance;
}

EventId UniqueEventIdFactory::NewUniqueEventId() {
  std::lock_guard<std::recursive_mutex> lock(mutex_);
  constexpr uint64_t kOne = 1;
  if (counter_ == 64) {
    Log::Fatal("UniqueEventIdFactory",
               "BioDynaMo only supports 64 unique EventIds."
               " You requested a 65th one.");
  }
  return kOne << counter_++;
}

UniqueEventIdFactory::UniqueEventIdFactory() {}

// -----------------------------------------------------------------------------
Event::~Event() {}

}  // namespace bdm
