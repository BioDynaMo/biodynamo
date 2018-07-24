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

#ifndef COMMIT_OP_H_
#define COMMIT_OP_H_

#include <unordered_map>
#include <vector>

namespace bdm {

class CommitOp {
 public:
  CommitOp() {}
  ~CommitOp() {}

  template <typename TSoContainer>
  void operator()(TSoContainer* sim_objects, uint16_t type_idx) {
    update_info_.emplace_back(sim_objects->Commit());
  }

  void Reset() { update_info_.clear(); }

  auto& GetUpdateInfo() const { return update_info_; }

 private:
  std::vector<std::unordered_map<uint32_t, uint32_t>> update_info_;
};

}  // namespace bdm

#endif  // COMMIT_OP_H_
