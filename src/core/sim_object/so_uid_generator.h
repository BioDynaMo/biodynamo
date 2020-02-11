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

#ifndef CORE_SIM_OBJECT_SO_UID_GENERATOR_H_
#define CORE_SIM_OBJECT_SO_UID_GENERATOR_H_

#include <atomic>
#include "core/sim_object/so_uid.h"
#include "core/util/root.h"

namespace bdm {

/// This class generates unique ids for simulation objects events satisfying the
/// EventId invariant. Thread safe.
class SoUidGenerator {
 public:
  SoUidGenerator(const SoUidGenerator&) = delete;
  SoUidGenerator() : counter_(0) {}

  SoUid NewSoUid() { return SoUid(counter_++); }

  // Returns the highest index that was used for a SoUid
  uint64_t GetHighestIndex() const { return counter_; }

 private:
  std::atomic<typename SoUid::Index_t> counter_;  //!
  /// ROOT can't persist std::atomic.
  /// Therefore this additional helper variable is needed.
  typename SoUid::Index_t root_counter_;
  BDM_CLASS_DEF_NV(SoUidGenerator, 1);
};

inline void SoUidGenerator::Streamer(TBuffer &R__b) {
  // Stream an object of class Foo.
  if (R__b.IsReading()) {
    R__b.ReadClassBuffer(SoUidGenerator::Class(), this);
    this->counter_ = this->root_counter_;
  } else {
    this->root_counter_ = this->counter_.load();
    R__b.WriteClassBuffer(SoUidGenerator::Class(), this);
  }
}


}  // namespace bdm

#endif  // CORE_SIM_OBJECT_SO_UID_GENERATOR_H_
