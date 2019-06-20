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

#ifndef CORE_UTIL_DEBUG_H_
#define CORE_UTIL_DEBUG_H_

#include <algorithm>
#include <array>
#include <iomanip>
#include <iostream>

#include "core/container/math_array.h"

inline void Print(const bdm::Double3& a, int precision = 10) {
  std::cout << std::setprecision(precision) << a[0] << ", " << a[1] << ", "
            << a[2] << std::endl;
}

#endif  // CORE_UTIL_DEBUG_H_
