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

namespace bdm {

class SimObject;

/// An Operation contains a function that will be executed for each simulation
/// object. It's data member `frequency_` specifies how often it will be
/// executed (every simulation step, every second, ...).
struct Operation {
  using FunctionType = std::function<void(SimObject*)>;

  Operation();

  Operation(const std::string& name, const FunctionType& f);

  Operation(const std::string& name, uint32_t frequency, const FunctionType& f);

  void operator()(SimObject* so) const;

  /// Specifies how often this operation will be executed.\n
  /// 1: every timestep\n
  /// 2: every second timestep\n
  /// ...
  uint32_t frequency_ = 1;
  /// Operation name / unique identifier
  std::string name_;

 private:
  FunctionType function_;
};

}  // namespace bdm

#endif  // CORE_OPERATION_OPERATION_H_
