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

#include "core/biology_module/biology_module.h"
#include "core/sim_object/sim_object.h"

namespace bdm {

BaseBiologyModule::BaseBiologyModule() : copy_mask_(0), remove_mask_(0) {}

BaseBiologyModule::BaseBiologyModule(EventId copy_event, EventId remove_event)
    : copy_mask_(copy_event), remove_mask_(remove_event) {}

BaseBiologyModule::BaseBiologyModule(std::initializer_list<EventId> copy_events,
                  std::initializer_list<EventId> remove_events) {
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

BaseBiologyModule::BaseBiologyModule(const Event& event, BaseBiologyModule* other, uint64_t new_oid) {
  copy_mask_ = other->copy_mask_;
  remove_mask_ = other->remove_mask_;
}

BaseBiologyModule::BaseBiologyModule(const BaseBiologyModule& other)
    : copy_mask_(other.copy_mask_), remove_mask_(other.remove_mask_) {}

 BaseBiologyModule::~BaseBiologyModule() {}

 void BaseBiologyModule::EventHandler(const Event &event, BaseBiologyModule *other1, BaseBiologyModule* other2) {}

bool BaseBiologyModule::Copy(EventId event) const { return (event & copy_mask_) != 0; }

bool BaseBiologyModule::Remove(EventId event) const { return (event & remove_mask_) != 0; }

}  // namespace bdm
