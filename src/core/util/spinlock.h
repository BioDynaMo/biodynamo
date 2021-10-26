// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & University of Surrey for the benefit of the
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

#ifndef CORE_UTIL_SPINLOCK_H_
#define CORE_UTIL_SPINLOCK_H_

#include <atomic>

namespace bdm {

class Spinlock {
 public:
  Spinlock() {}

  /// Used to store mutexes in a std::vector.
  /// Always creates a new mutex (even for the copy constructor)
  Spinlock(const Spinlock& other) {}

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
  std::atomic_flag flag_ = ATOMIC_FLAG_INIT;  //!
};

}  // namespace bdm

#endif  // CORE_UTIL_SPINLOCK_H_
