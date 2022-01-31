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

#ifndef CORE_LOAD_BALANCE_INFO_H_
#define CORE_LOAD_BALANCE_INFO_H_

#include "core/agent/agent_handle.h"
#include "core/functor.h"
#include "core/util/iterator.h"

namespace bdm {

class LoadBalanceInfo {
 public:
  virtual ~LoadBalanceInfo() {}
  virtual void CallHandleIteratorConsumer(
      uint64_t start, uint64_t end,
      Functor<void, Iterator<AgentHandle>*>& f) const = 0;
};

}  // namespace bdm

#endif  // CORE_LOAD_BALANCE_INFO_H_
