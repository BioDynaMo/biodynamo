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

template <typename TCTParam = CompileTimeParam<>>
class ApproximateExecCtxt {
public:
  void SetupIteration() {
    remove_.clear();
  }

  template <typename TSimulation = Simulation<>>
  void TearDownIteration() {
    auto* rm = TSimulation::GetActive()->GetResourceManager();
    for(auto& uid : remove_) {
      rm->Remove(uid);
    }
    remove_.clear();
  }
  //
  // template <typename TSo, typename TSimBackend = Backend, typename TSimulation = Simulation<>>
  // auto&& GetSimObject(SoUid uid, typename std::enable_if<std::is_same<TSimBackend, Scalar>::value>::type* ptr = 0) {
  //   // check if the uid correspons to a new object not yet in the Rm
  //
  //   auto* rm = TSimulation::GetActive()->GetResourceManager();
  //   return rm->template GetSimObject<TSo>(uid_);
  // }

  // template <typename TSo, typename TSimBackend = Backend>
  // auto GetSimObject(SoUid uid, typename std::enable_if<std::is_same<TSimBackend, Soa>::value>::type* ptr = 0) {
  //   auto handle = so_storage_location_[uid];
  //   return (*Get<TSo>())[handle.GetElementIdx()];
  // }

  template <typename TSimulation = Simulation<>>
  void RemoveFromSimulation(SoUid uid) {
    auto* rm = TSimulation::GetActive()->GetResourceManager();
    remove_.push_back(uid);
  }

private:
  std::vector<SoUid> remove_;
};

}  // namespace bdm

#endif  // EXECUTION_CONTEXT_APPROXIMATE_EXECUTION_CTXT_H_
