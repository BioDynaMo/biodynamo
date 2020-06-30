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
#include "core/sim_object/sim_object.h"

class TClass;

namespace bdm {

struct TypeIndex {
  void Add(SimObject* so);

  void Update(SimObject* new_so);
  
  void Remove(SimObject* so);

  void Clear();

  void Reserve(uint64_t capacity);

  const std::vector<SimObject*>& GetType(TClass* tclass) const;

private:
  UnorderedFlatmap<TClass*, std::vector<SimObject*>> data_;
  SoUidMap<uint64_t> index_;
};

}  // namespace bdm

#endif  // CORE_TYPE_INDEX_H_
