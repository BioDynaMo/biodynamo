// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & Newcastle University for the benefit of the
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

#include "core/util/thread_info.h"

namespace bdm {

std::atomic<uint64_t> ThreadInfo::thread_counter_;

ThreadInfo* ThreadInfo::GetInstance() {
  static ThreadInfo kInstance;
  return &kInstance;
}

uint64_t ThreadInfo::GetUniversalThreadId() const {
  thread_local uint64_t kTid = thread_counter_++;
  return kTid;
}

}  // namespace bdm
