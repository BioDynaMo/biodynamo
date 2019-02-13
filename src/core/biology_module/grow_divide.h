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

#ifndef CORE_BIOLOGY_MODULE_GROW_DIVIDE_H_
#define CORE_BIOLOGY_MODULE_GROW_DIVIDE_H_

#include "core/biology_module/biology_module.h"
#include "core/util/root.h"

namespace bdm {

/// This biology module grows the simulation object until the diameter reaches
/// the specified threshold and divides the object afterwards.
struct GrowDivide : public BaseBiologyModule {
  GrowDivide();
  GrowDivide(double threshold, double growth_rate,
             std::initializer_list<EventId> event_list);

  GrowDivide(const Event& event, BaseBiologyModule* other, uint64_t new_oid = 0);

  virtual ~GrowDivide();

  /// Create a new instance of this object using the default constructor.
  BaseBiologyModule* GetInstance(const Event& event, BaseBiologyModule* other, uint64_t new_oid = 0) const override;

  /// Create a copy of this biology module.
  BaseBiologyModule* GetCopy() const override;

  /// Default event handler (exising biology module won't be modified on
  /// any event)
  void EventHandler(const Event &event, BaseBiologyModule *other1, BaseBiologyModule* other2 = nullptr) override;

  void Run(SimObject* so) override;

 private:
  BDM_CLASS_DEF_OVERRIDE(GrowDivide, 1);
  double threshold_ = 40;
  double growth_rate_ = 300;
};

}  // namespace bdm

#endif  // CORE_BIOLOGY_MODULE_GROW_DIVIDE_H_
