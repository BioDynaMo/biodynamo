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

#ifndef CORE_FEN_FUNC_H_
#define CORE_FEN_FUNC_H_

namespace bdm {

struct FenFunc {
  virtual void operator()(const SimObject* so, double squared_distance) = 0;
};

}  // namespace bdm

#endif  // CORE_FEN_FUNC_H_
