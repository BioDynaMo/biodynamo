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
#ifndef CORE_REAL_H_
#define CORE_REAL_H_

namespace bdm {

#ifndef BDM_REALT
using real_t = float;
#else
using real_t = BDM_REALT;
#endif  // BDM_REALT

}  // namespace bdm

#endif  // CORE_REAL_H_
