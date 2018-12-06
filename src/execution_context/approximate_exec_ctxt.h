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

#ifndef EXECUTION_CONTEXT_APPROXIMATE_EXEC_CTXT_H_
#define EXECUTION_CONTEXT_APPROXIMATE_EXEC_CTXT_H_

#include <vector>

namespace bdm {

class ApproximateExecCtxt {
public:
  void SetupIteration() {
    remove_.clear();
  }

  template <typename TSimulation = Simulation<>>
  void RemoveFromSimulation(SoUid uid) {
    auto* rm = TSimulation::GetActive()->GetResourceManager();
    remove_.push_back(uid);
  }

  template <typename TSimulation = Simulation<>>
  void TearDownIteration() {
    auto* rm = TSimulation::GetActive()->GetResourceManager();
    for(auto& uid : remove_) {
      rm->Remove(uid);
    }
    remove_.clear();
  }

private:
  std::vector<SoUid> remove_;
};

}  // namespace bdm

#endif  // EXECUTION_CONTEXT_APPROXIMATE_EXECUTION_CTXT_H_
