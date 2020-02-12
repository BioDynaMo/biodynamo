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
#include <mutex>
#include "core/container/so_uid_map.h"
#include "core/sim_object/so_handle.h"
#include "core/sim_object/so_uid.h"
#include "core/util/root.h"
#include "core/util/spinlock.h"
#include "core/simulation.h"
#include "core/scheduler.h"

namespace bdm {

/// This class generates unique ids for simulation objects.
/// All functions are  thread safe.
class SoUidGenerator {
 public:
  SoUidGenerator(const SoUidGenerator&) = delete;
  SoUidGenerator() : counter_(0) {}

  /// Generates SoUid with increasing index.
  /// In defragmentation mode it resuses index values from removed sim objects
  /// and sets the reused field to the current simulation step.
  SoUid NewSoUid() {
    if (map_ != nullptr) {
      // defragmentation mode
      std::lock_guard<Spinlock> guard(lock_);
      while (search_index_ != map_->size() &&
        map_->GetReused(search_index_) != std::numeric_limits<typename SoUid::Reused_t>::max()){
        search_index_++;
      }
      // find unused element in map
      if(search_index_ < map_->size()) {
        auto* scheduler = Simulation::GetActive()->GetScheduler();
        return SoUid(search_index_++, scheduler->GetSimulatedSteps());
      }
      // didn't find any empty slots -> disable defragmentation mode
      map_ = nullptr;
    }
    return SoUid(counter_++);
  }

  // Returns the highest index that was used for a SoUid
  uint64_t GetHighestIndex() const { return counter_; }

  void EnableDefragmentation(const SoUidMap<SoHandle>* map) {
    map_ = map;
    search_index_ = 0;
  }

  void DisableDefragmentation() { map_ = nullptr; }

  bool IsInDefragmentationMode() const { return map_ != nullptr; }

 private:
  std::atomic<typename SoUid::Index_t> counter_;  //!
  /// ROOT can't persist std::atomic.
  /// Therefore this additional helper variable is needed.
  typename SoUid::Index_t root_counter_;

  ///
  const SoUidMap<SoHandle>* map_ = nullptr;  //!
  /// Lock needed for defragmentation mode
  Spinlock lock_;  //!
  /// Current search position
  typename SoUid::Index_t search_index_ = 0;  //!
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
