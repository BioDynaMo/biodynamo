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

#ifndef CORE_TYPE_INDEX_H_
#define CORE_TYPE_INDEX_H_

#include <vector>
#include "core/container/flatmap.h"
#include "core/agent/agent.h"

class TClass;

namespace bdm {

struct TypeIndex {
  void Add(Agent* agent);

  void Update(Agent* new_so);

  void Remove(Agent* agent);

  void Clear();

  void Reserve(uint64_t capacity);

  const std::vector<Agent*>& GetType(TClass* tclass) const;

 private:
  UnorderedFlatmap<TClass*, std::vector<Agent*>> data_;
  AgentUidMap<uint64_t> index_;
};

}  // namespace bdm

#endif  // CORE_TYPE_INDEX_H_
