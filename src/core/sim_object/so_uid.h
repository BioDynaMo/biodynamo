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

#ifndef CORE_SIM_OBJECT_SO_UID_H_
#define CORE_SIM_OBJECT_SO_UID_H_

#include <atomic>

namespace bdm {

/// SoUid is a unique id for simulation objects that remains unchanged
/// throughout the whole simulation.
using SoUid = uint64_t;

/// This class generates unique ids for simulation objects events satisfying the
/// EventId invariant. Thread safe.
class SoUidGenerator {
 public:
  SoUidGenerator(const SoUidGenerator&) = delete;

  static SoUidGenerator* Get() {
    static SoUidGenerator kInstance;
    return &kInstance;
  }

  SoUid NewSoUid() { return counter_++; }

  SoUid GetLastId() const { return counter_; }

 private:
  SoUidGenerator() : counter_(0) {}
  std::atomic<SoUid> counter_;
};

}  // namespace bdm

#endif  // CORE_SIM_OBJECT_SO_UID_H_
