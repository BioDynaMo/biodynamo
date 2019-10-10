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

#ifndef CORE_UTIL_SPINLOCK_H_
#define CORE_UTIL_SPINLOCK_H_

#include <atomic>

class Spinlock {
 public:
  Spinlock() {}

  void lock() {  // NOLINT
    while (flag_.test_and_set()) {
      // spin
      ;
    }
  }

  void unlock() {  // NOLINT
    flag_.clear();
  }

 private:
  std::atomic_flag flag_ = ATOMIC_FLAG_INIT;
};

#endif  // CORE_UTIL_SPINLOCK_H_
