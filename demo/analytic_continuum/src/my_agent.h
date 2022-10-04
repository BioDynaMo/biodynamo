// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & University of Surrey for the benefit of the
// BioDynaMo collaboration. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#include <cmath>
#include "biodynamo.h"

#ifndef MY_AGENT_H_
#define MY_AGENT_H_

namespace bdm {

/// A simple agent that can sense the continuum value and stores the value in
/// a member variable.
class ContinuumRetrieverAgent : public SphericalAgent {
  BDM_AGENT_HEADER(ContinuumRetrieverAgent, SphericalAgent, 1);

 public:
  ContinuumRetrieverAgent() {}
  explicit ContinuumRetrieverAgent(const Real3& position) : Base(position) {}
  virtual ~ContinuumRetrieverAgent() {}

  real_t GetMyContinuumValue() const { return my_continuum_value_; }
  void SetMyContinuumValue(real_t continuum_value) {
    my_continuum_value_ = continuum_value;
  }

 private:
  /// This member stores the value of the continuum at the agent's position.
  real_t my_continuum_value_ = 0.0;
};

}  // namespace bdm

#endif  // MY_AGENT_H_
