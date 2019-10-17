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

#ifndef CORE_CONTAINER_UNORDERED_SPLIT_FLATMAP_H_
#define CORE_CONTAINER_UNORDERED_SPLIT_FLATMAP_H_

#include "core/container/inline_vector.h"

namespace bdm {

template <typename TKey, typename TValue, uint64_t TSize>
class UnorderedSplitFlatMap {
 public:
  UnorderedSplitFlatMap() {}

  TValue& operator[](const TKey& key) {}

  const TValue& at(const TKey& key) const {  // NOLINT
  }

 private:
  InlineVector<TKey, TSize> keys_;
  InlineVector<TValue, TSize> values_;
};

}  // namespace bdm

#endif  // CORE_CONTAINER_UNORDERED_SPLIT_FLATMAP_H_
