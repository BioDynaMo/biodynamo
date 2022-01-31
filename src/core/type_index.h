// -----------------------------------------------------------------------------
//
// Copyright (C) 2022 CERN & University of Surrey for the benefit of the
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

#ifndef CORE_TYPE_INDEX_H_
#define CORE_TYPE_INDEX_H_

#include <vector>
#include "core/agent/agent.h"
#include "core/container/flatmap.h"

class TClass;

namespace bdm {

class TypeIndex {
 public:
  void Add(Agent* agent);

  void Update(Agent* new_agent);

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
