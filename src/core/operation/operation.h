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

#ifndef CORE_OPERATION_OPERATION_H_
#define CORE_OPERATION_OPERATION_H_

#include <functional>
#include <string>
#include "core/functor.h"
#include <iostream>

namespace bdm {

class SimObject;

/// An Operation contains a function that will be executed for each simulation
/// object. It's data member `frequency_` specifies how often it will be
/// executed (every simulation step, every second, ...).
struct Operation : public Functor<void, SimObject*> {
  Operation(); // FIXME remove

  Operation(const std::string& name);

  Operation(const std::string& name, uint32_t frequency);

  void operator()(SimObject* so) override {} // FIXME remove to make abstract

  /// Specifies how often this operation will be executed.\n
  /// 1: every timestep\n
  /// 2: every second timestep\n
  /// ...
  uint32_t frequency_ = 1;
  /// Operation name / unique identifier
  std::string name_;
};

}  // namespace bdm

#endif  // CORE_OPERATION_OPERATION_H_
